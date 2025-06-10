from ultralytics import YOLO
import cv2
import requests
import threading
import time

ESP32_IP = "http://192.168.4.1"
STREAM_URL = f"{ESP32_IP}:81/stream"
model = YOLO('yolov8n.pt')

CONFIDENCE_THRESHOLD = 0.6
FIRE_CLASSES = ['fire', 'flame', 'smoke']
OBSTACLE_CLASSES = ['person', 'chair', 'table', 'car', 'sofa']

frame_lock = threading.Lock()
latest_frame = None
stop_thread = False

def capturar_stream():
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
        detections.append({
            'class': cls_name,
            'conf': conf,
            'box': [int(x1), int(y1), int(x2), int(y2)],
        })
    return detections

def escolher_foco_fogo(detections):
    fogos = [d for d in detections if d['class'] in FIRE_CLASSES]
    if not fogos:
        return None
    return max(fogos, key=lambda f: (f['box'][2] - f['box'][0]) * (f['box'][3] - f['box'][1]))

def obstaculo_bloqueando(fogo, detections):
    fogo_cx = (fogo['box'][0] + fogo['box'][2]) // 2
    fogo_cy = (fogo['box'][1] + fogo['box'][3]) // 2
    obstaculos = [d for d in detections if d['class'] in OBSTACLE_CLASSES]
    for obs in obstaculos:
        x1, y1, x2, y2 = obs['box']
        if x1 < fogo_cx < x2 and y1 < fogo_cy < y2:
            return obs
    return None

def decidir_direcao_desvio(obstaculo, frame_largura):
    x1, _, x2, _ = obstaculo['box']
    obs_cx = (x1 + x2) // 2

    if obs_cx < frame_largura // 2:
        return "direita"
    else:
        return "esquerda"

def enviar_comando_direcao(direcao):
    try:
        if direcao == "esquerda":
            url = f"{ESP32_IP}/desviar_esquerda"
        elif direcao == "direita":
            url = f"{ESP32_IP}/desviar_direita"
        else:
            url = f"{ESP32_IP}/recuar"
        r = requests.get(url, timeout=1.5)
        print(f"Desviando para {direcao}:", r.text)
    except Exception as e:
        print("Erro ao enviar comando de desvio:", e)

def acionar_extintor():
    try:
        r = requests.get(f"{ESP32_IP}/apagar", timeout=1.5)
        print("Extintor acionado:", r.text)
    except Exception as e:
        print("Erro ao acionar extintor:", e)

thread = threading.Thread(target=capturar_stream)
thread.start()

frame_count = 0
ultima_deteccao = []

try:
    while True:
        with frame_lock:
            frame = latest_frame.copy() if latest_frame is not None else None

        if frame is None:
            time.sleep(0.01)
            continue

        frame_count += 1
        if frame_count % 10 == 0:
            ultima_deteccao = detectar_objetos(frame)
            foco_fogo = escolher_foco_fogo(ultima_deteccao)

            if foco_fogo:
                obstaculo = obstaculo_bloqueando(foco_fogo, ultima_deteccao)
                if obstaculo:
                    largura_frame = frame.shape[1]
                    direcao = decidir_direcao_desvio(obstaculo, largura_frame)
                    enviar_comando_direcao(direcao)
                else:
                    acionar_extintor()
        else:
            foco_fogo = escolher_foco_fogo(ultima_deteccao)

        # Exibição das detecções
        for d in ultima_deteccao:
            x1, y1, x2, y2 = d['box']
            color = (0, 0, 255) if d['class'] in FIRE_CLASSES else (255, 0, 0)
            cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
            cv2.putText(frame, f"{d['class']} {d['conf']:.2f}", (x1, y1 - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)

        if foco_fogo:
            x1, y1, x2, y2 = foco_fogo['box']
            cv2.putText(frame, ">> FOCO PRINCIPAL", (x1, y2 + 20),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

        cv2.imshow("RoboFIRE Visão IA", frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except KeyboardInterrupt:
    pass

stop_thread = True
thread.join()
cv2.destroyAllWindows()
