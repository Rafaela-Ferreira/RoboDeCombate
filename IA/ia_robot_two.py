from ultralytics import YOLO
import cv2
import requests
import threading
import time

# ===== CONFIGURAÇÕES =====
ESP32_IP = "http://192.168.4.1"
STREAM_URL = f"{ESP32_IP}:81/stream"
model = YOLO("fire-detection-yolov8.pt")

CONFIDENCE_THRESHOLD = 0.6

FIRE_CLASSES = ["fire", "flame", "smoke"]
OBSTACLE_CLASSES = ["person", "chair", "table", "car", "sofa"]

# ===== VARIÁVEIS =====
frame_lock = threading.Lock()
latest_frame = None
stop_thread = False

frame_count = 0
ultima_deteccao = []
frames_com_fogo = 0
frames_sem_fogo = 0
FOGO_FRAMES_MIN = 3
FOGO_FRAME_RESET = 5

# ===== THREAD DE CAPTURA =====
def capturar_stream():
    global latest_frame, stop_thread
    cap = cv2.VideoCapture(STREAM_URL)
    if not cap.isOpened():
        print("❌ Erro: não foi possível conectar ao stream da ESP32-CAM.")
        stop_thread = True
        return

    while not stop_thread:
        ret, frame = cap.read()
        if ret:
            with frame_lock:
                latest_frame = frame.copy()
        else:
            print("⚠️ Erro ao capturar frame")
        time.sleep(0.01)
    cap.release()

# ===== DETECÇÃO =====
def detectar_objetos(frame):
    results = model(frame, verbose=False)[0]
    detections = []
    for r in results.boxes.data.tolist():
        x1, y1, x2, y2, conf, cls = r
        if conf < CONFIDENCE_THRESHOLD:
            continue
        cls_name = model.names[int(cls)]
        detections.append({
            "class": cls_name,
            "conf": conf,
            "box": [int(x1), int(y1), int(x2), int(y2)],
        })
    return detections

def escolher_foco_fogo(detections):
    fogos = [d for d in detections if d["class"] in FIRE_CLASSES]
    if not fogos:
        return None
    return max(fogos, key=lambda f: (f["box"][2] - f["box"][0]) * (f["box"][3] - f["box"][1]))

def obstaculo_bloqueando(fogo, detections):
    fogo_cx = (fogo["box"][0] + fogo["box"][2]) // 2
    fogo_cy = (fogo["box"][1] + fogo["box"][3]) // 2
    for obs in detections:
        if obs["class"] not in OBSTACLE_CLASSES:
            continue
        x1, y1, x2, y2 = obs["box"]
        if x1 < fogo_cx < x2 and y1 < fogo_cy < y2:
            return obs
    return None

def decidir_direcao_desvio(obstaculo, frame_largura):
    x1, _, x2, _ = obstaculo["box"]
    obs_cx = (x1 + x2) // 2
    return "direita" if obs_cx < frame_largura // 2 else "esquerda"

# ===== CONTROLE DO ROBÔ =====
def enviar_comando(endpoint):
    try:
        r = requests.get(f"{ESP32_IP}/{endpoint}", timeout=1.5)
        print(f"[CMD] {endpoint} → {r.text}")
    except Exception as e:
        print("⚠️ Erro ao enviar comando:", e)

def enviar_comando_direcao(direcao):
    if direcao == "esquerda":
        enviar_comando("desviar_esquerda")
    elif direcao == "direita":
        enviar_comando("desviar_direita")
    else:
        enviar_comando("recuar")

def acionar_extintor():
    enviar_comando("apagar")

def parar_robo():
    enviar_comando("parar")

# ===== MAIN LOOP =====
thread = threading.Thread(target=capturar_stream)
thread.start()

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
                frames_com_fogo += 1
                frames_sem_fogo = 0
                if frames_com_fogo >= FOGO_FRAMES_MIN:
                    obstaculo = obstaculo_bloqueando(foco_fogo, ultima_deteccao)
                    if obstaculo:
                        largura_frame = frame.shape[1]
                        direcao = decidir_direcao_desvio(obstaculo, largura_frame)
                        enviar_comando_direcao(direcao)
                    else:
                        largura = foco_fogo["box"][2] - foco_fogo["box"][0]
                        altura = foco_fogo["box"][3] - foco_fogo["box"][1]
                        area = largura * altura
                        if area > 20000:
                            acionar_extintor()
                        else:
                            enviar_comando("avancar")
            else:
                frames_sem_fogo += 1
                if frames_sem_fogo >= FOGO_FRAME_RESET:
                    frames_com_fogo = 0
                    parar_robo()

        # ==== Exibição =====
        for d in ultima_deteccao:
            x1, y1, x2, y2 = d["box"]
            color = (0, 0, 255) if d["class"] in FIRE_CLASSES else (255, 0, 0)
            cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
            cv2.putText(frame, f"{d['class']} {d['conf']:.2f}", (x1, y1 - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)

        foco_fogo = escolher_foco_fogo(ultima_deteccao)
        if foco_fogo:
            x1, y1, x2, y2 = foco_fogo["box"]
            cv2.putText(frame, ">> FOCO PRINCIPAL", (x1, y2 + 20),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

        cv2.imshow("RoboFIRE Visão IA", frame)

        if cv2.waitKey(1) & 0xFF == ord("q"):
            break

except KeyboardInterrupt:
    pass

stop_thread = True
thread.join()
cv2.destroyAllWindows()