from ultralytics import YOLO
import cv2
import requests
import threading
import time

ESP32_IP = "http://192.168.4.1"
STREAM_URL = f"{ESP32_IP}:81/stream"
model = YOLO('yolov8n.pt')

FIRE_CLASSES = ['fire', 'flame']
OBSTACLE_CLASSES = ['person', 'chair', 'table', 'car']
CONFIDENCE_THRESHOLD = 0.6

frame_lock = threading.Lock()
latest_frame = None
stop_thread = False

def capture_thread_func():
    global latest_frame, stop_thread
    cap = cv2.VideoCapture(STREAM_URL)
    if not cap.isOpened():
        print("Erro: não foi possível conectar ao stream da ESP32-CAM.")
        stop_thread = True
        return

    while not stop_thread:
        ret, frame = cap.read()
        if ret:
            with frame_lock:
                latest_frame = frame.copy()
        else:
            print("Erro ao capturar frame")
        time.sleep(0.01)

    cap.release()

def detectar_objetos(frame):
    results = model(frame, verbose=False)[0]
    detections = []

    for r in results.boxes.data.tolist():
        x1, y1, x2, y2, conf, cls = r
        if conf < CONFIDENCE_THRESHOLD:
            continue

        cls_name = model.names[int(cls)]
        print(f"Detectado: {cls_name} com confiança {conf:.2f}")  # Debug opcional
        detections.append({
            'class': cls_name,
            'conf': conf,
            'box': [int(x1), int(y1), int(x2), int(y2)],
        })
    return detections


def escolher_foco_fogo(detections):
    fogos = [d for d in detections if d['class'] in FIRE_CLASSES]
    obstaculos = [d for d in detections if d['class'] in OBSTACLE_CLASSES]

    if not fogos:
        return None

    foco_escolhido = max(fogos, key=lambda f: (f['box'][2] - f['box'][0]) * (f['box'][3] - f['box'][1]))
    fogo_cx = (foco_escolhido['box'][0] + foco_escolhido['box'][2]) // 2
    fogo_cy = (foco_escolhido['box'][1] + foco_escolhido['box'][3]) // 2

    for obs in obstaculos:
        x1, y1, x2, y2 = obs['box']
        if x1 < fogo_cx < x2 and y1 < fogo_cy < y2:
            return None
    return foco_escolhido

def acionar_extintor():
    try:
        r = requests.get(f"{ESP32_IP}/apagar", timeout=1.5)
        print("Extintor acionado:", r.text)
    except:
        print("Erro ao acionar extintor")

thread = threading.Thread(target=capture_thread_func)
thread.start()

frame_count = 0
detections = []

try:
    while True:
        with frame_lock:
            frame = latest_frame.copy() if latest_frame is not None else None

        if frame is None:
            time.sleep(0.01)
            continue

        frame_count += 1

        if frame_count % 10 == 0:
            detections = detectar_objetos(frame)
            foco = escolher_foco_fogo(detections)
            if foco:
                acionar_extintor()
        else:
            foco = escolher_foco_fogo(detections)

        for d in detections:
            x1, y1, x2, y2 = d['box']
            color = (0, 0, 255) if d['class'] in FIRE_CLASSES else (255, 0, 0)
            cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
            cv2.putText(frame, f"{d['class']} {d['conf']:.2f}", (x1, y1 - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)

        if foco:
            x1, y1, x2, y2 = foco['box']
            cv2.putText(frame, ">> PRIORITÁRIO", (x1, y2 + 20),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

        cv2.imshow("RoboFIRE Visão", frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except KeyboardInterrupt:
    pass

stop_thread = True
thread.join()
cv2.destroyAllWindows()