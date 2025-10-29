import smtplib
import time 
from email.mime.text import MIMEText

SENDER_EMAIL = "jpbleeden@gmail.com"
APP_PASSWORD = "lfmeifzirtaycmml"
RECIPIENT_SMS = "3195699155@vtext.com"  # Verizon SMS gateway

def send_text_alert():
    try:
        # Create message # Critical Safety Event at HH:MM XX on Month/Day/Year
        timestamp = time.time()
        msg = MIMEText(f"Critical Safety Event at {time.strftime('%H:%M', time.localtime(timestamp))} on {time.strftime('%m/%d/%Y', time.localtime(timestamp))}")
        msg["Subject"] = "IR Alert"
        msg["From"] = SENDER_EMAIL
        msg["To"] = RECIPIENT_SMS

        # Send via Gmail SMTP
        with smtplib.SMTP_SSL("smtp.gmail.com", 465) as server:
            server.login(SENDER_EMAIL, APP_PASSWORD)
            server.send_message(msg)

        print("Alert message sent successfully.")
        return True

    except Exception as e:
        print("ERROR: Failed to send alert:", e)
        return False
