import requests
import time
from messageService import send_text_alert

ESP32_URL = "http://192.168.1.1/data" 
POLL_INTERVAL = 1.1   # seconds between status checks
ALERT_COOLDOWN = 60 # seconds between text alerts

def poll_esp32():
    last_alert_time = 0
    print("Starting ESP32 polling...")

    while True:
        try:
            response = requests.get(ESP32_URL, timeout=3)
            
            if response.status_code == 200:
                data = response.json()
                print("Received:", data)

                # Example JSON your ESP32 might return:
                # {"signal_blocked": true}
                if data.get("signal_blocked"):
                    if time.time() - last_alert_time > ALERT_COOLDOWN:
                        print("Signal blocked — sending SMS...")
                        success = send_text_alert()
                        if success:
                            last_alert_time = time.time()
                else:
                    print("IR beam detected.")
            else:
                print(f"Bad response: {response.status_code}")

        except requests.exceptions.RequestException as e:
            print("Connection error:", e)

        time.sleep(POLL_INTERVAL)

# --- Main ---
if __name__ == "__main__":
    try:
        poll_esp32()
    except KeyboardInterrupt:
        print("\nStopping polling script.")
