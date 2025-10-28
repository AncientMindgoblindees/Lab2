"""
alertClient.py
--------------
Polls the ESP32 web server (AP mode) for sensor data.
When signal_lock == 0 (beam blocked), sends a text alert via messageService.
"""

import requests
import time
from messageService import send_text_alert

# --- Configuration ---
ESP32_URL = "http://192.168.1.1/data"
POLL_INTERVAL = 2       # seconds between checks
ALERT_COOLDOWN = 60     # seconds between alert messages

def poll_esp32():
    last_alert_time = 0
    print("[INFO] Polling ESP32 for IR status...")

    while True:
        try:
            r = requests.get(ESP32_URL, timeout=3)
            if r.status_code == 200:
                data = r.json()
                print("[DEBUG] Received:", data)

                lock_state = data.get("signal_lock")
                if lock_state == 0:   # Beam blocked
                    if time.time() - last_alert_time > ALERT_COOLDOWN:
                        print("[ALERT] IR beam interrupted — sending SMS...")
                        send_text_alert()
                        last_alert_time = time.time()
                else:
                    print("[OK] IR beam detected.")
            else:
                print(f"[WARN] Bad response: {r.status_code}")

        except requests.exceptions.RequestException as e:
            print("[ERR] Connection error:", e)

        time.sleep(POLL_INTERVAL)

if __name__ == "__main__":
    try:
        poll_esp32()
    except KeyboardInterrupt:
        print("\n[INFO] Stopping polling.")
