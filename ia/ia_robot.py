from ultralytics import YOLO
import cv2
import time
import threading

# =============================
# CONFIGURAÇÕES
# =============================
USE_ESP32 = False
ESP32_IP = "http://192.168.4.1"
STREAM_URL = f"{ESP32_IP}"

VIDEO_TEST = "./teste_incendio.mp4"  # Usar 0 (caso o uso do ESP for True) ou o caminho (caso o uso do ESP for False)

model = YOLO("best.pt")

CONFIDENCE_THRESHOLD = 0.6
FIRE_CLASSES = ["fire", "flame", "smoke"]
OBSTACLE_CLASSES = ["person", "chair", "table", "car", "sofa"]

# =============================
# AJUSTE DE VELOCIDADE DO VÍDEO
# =============================
VIDEO_SPEED = 3.0
FRAME_DELAY = 0.01 * VIDEO_SPEED

# =============================
# VARIÁVEIS
# =============================
frame_lock = threading.Lock()
latest_frame = None
stop_thread = False

frame_count = 0
ultima_deteccao = []
frames_com_fogo = 0
frames_sem_fogo = 0

FOGO_FRAMES_MIN = 3
FOGO_FRAME_RESET = 5

ultima_msg = ""


def log(msg):
    """Exibe mensagem somente se for diferente da última (anti-spam)."""
    global ultima_msg
    if msg != ultima_msg:
        print(msg)
        ultima_msg = msg


# ===========================
# CAPTURA DE VÍDEO
# ===========================

def capturar_stream():
    global latest_frame, stop_thread

    cap = cv2.VideoCapture(STREAM_URL if USE_ESP32 else VIDEO_TEST)

    if not cap.isOpened():
        print("❌ Erro: não conseguiu abrir stream da câmera.")
        stop_thread = True
        return

    while not stop_thread:
        ret, frame = cap.read()
        if not ret:
            break
            if not USE_ESP32:
                cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                continue

            log("⚠ Falha na captura... reconectando.")
            time.sleep(0.2)
            continue

        with frame_lock:
            latest_frame = frame.copy()

        time.sleep(FRAME_DELAY)

    cap.release()


# ===========================
# IA: DETECÇÃO
# ===========================

def detectar_objetos(frame):
    results = model(frame, verbose=False)[0]
    detections = []
    for r in results.boxes.data.tolist():
        x1, y1, x2, y2, conf, cls = r
        if conf < CONFIDENCE_THRESHOLD:
            continue
        detections.append({
            "class": model.names[int(cls)],
            "conf": conf,
            "box": [int(x1), int(y1), int(x2), int(y2)],
        })
    return detections


def escolher_foco_fogo(detections):
    fogos = [d for d in detections if d["class"] in FIRE_CLASSES]
    if not fogos:
        return None
    return max(fogos, key=lambda f: (f["box"][2]-f["box"][0])*(f["box"][3]-f["box"][1]))


def obstaculo_bloqueando(foco, detections):
    fogo_cx = (foco["box"][0] + foco["box"][2]) // 2
    fogo_cy = (foco["box"][1] + foco["box"][3]) // 2
    for obs in detections:
        if obs["class"] not in OBSTACLE_CLASSES:
            continue
        x1, y1, x2, y2 = obs["box"]
        if x1 < fogo_cx < x2 and y1 < fogo_cy < y2:
            return obs
    return None


def decidir_direcao_desvio(obstaculo, w):
    x1, _, x2, _ = obstaculo["box"]
    return "direita" if (x1 + x2) // 2 < w // 2 else "esquerda"


# ===========================
# MAIN LOOP
# ===========================

threading.Thread(target=capturar_stream).start()

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
            foco = escolher_foco_fogo(ultima_deteccao)

            if foco:
                frames_com_fogo += 1
                frames_sem_fogo = 0

                if frames_com_fogo >= FOGO_FRAMES_MIN:

                    obs = obstaculo_bloqueando(foco, ultima_deteccao)

                    if obs:
                        direcao = decidir_direcao_desvio(obs, frame.shape[1])
                        log(f"[IA] Obstáculo bloqueando o fogo → desviar para {direcao}")

                    else:
                        w = foco["box"][2] - foco["box"][0]
                        h = foco["box"][3] - foco["box"][1]
                        area = w * h

                        if area > 20000:
                            log("[IA] Fogo muito próximo → acionaria extintor")
                        else:
                            log("[IA] Aproximando do foco detectado...")

            else:
                frames_sem_fogo += 1
                if frames_sem_fogo >= FOGO_FRAME_RESET:
                    frames_com_fogo = 0
                    log("[IA] Nenhum fogo detectado")

        # EXIBIÇÃO
        for d in ultima_deteccao:
            x1, y1, x2, y2 = d["box"]
            color = (0, 0, 255) if d["class"] in FIRE_CLASSES else (255, 0, 0)
            cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
            cv2.putText(frame, f"{d['class']} {d['conf']:.2f}",
                        (x1, y1 - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)

        foco = escolher_foco_fogo(ultima_deteccao)
        if foco:
            x1, y1, x2, y2 = foco["box"]
            cv2.putText(frame, ">> FOCO PRINCIPAL",
                        (x1, y2 + 20),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6,
                        (0, 255, 0), 2)

        cv2.imshow("RoboFIRE Visão IA (Sem Comandos)", frame)

        if cv2.waitKey(1) & 0xFF == ord("q"):
            break

except KeyboardInterrupt:
    pass

stop_thread = True
cv2.destroyAllWindows()
