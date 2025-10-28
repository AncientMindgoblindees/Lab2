import requests, time
from messageService import send_text_alert

ESP32_URL = "http://192.168.1.1/data"
POLL_INTERVAL = 2        # seconds between polls
ALERT_COOLDOWN = 20      # seconds between text alerts

def poll_esp32():
    last_alert_time = 0
    beam_blocked = False   # tracks previous state

    print("[INFO] Polling ESP32 for IR status...")

    while True:
        try:
            r = requests.get(ESP32_URL, timeout=3)
            if r.status_code == 200:
                data = r.json()
                lock_state = data.get("signal_lock")

                # signal blocked
                if lock_state == 0:
                    if not beam_blocked:
                        # just transitioned to "blocked"
                        beam_blocked = True
                        last_alert_time = 0  # reset timer
                        print("[ALERT] Beam interrupted! Sending first alert...")
                        send_text_alert()
                        last_alert_time = time.time()
                    elif time.time() - last_alert_time > ALERT_COOLDOWN:
                        # still blocked and the cooldown expired, send again
                        print("[INFO] Beam still blocked, sending reminder alert...")
                        send_text_alert()
                        last_alert_time = time.time()

                # --- beam restored ---
                elif lock_state == 1:
                    if beam_blocked:
                        print("[OK] Beam restored.")
                    beam_blocked = False

                else:
                    print("[WARN] Unexpected data:", data)

            else:
                print(f"[WARN] Bad response: {r.status_code}")

        except requests.exceptions.RequestException as e:
            print("[ERR] Connection error:", e)

        time.sleep(POLL_INTERVAL)
