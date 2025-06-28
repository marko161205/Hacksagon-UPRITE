# Uprite Device & SmartStance Application

SmartStance is a cross-platform posture monitoring application built using **Flutter**. It monitors and classifies a user's back posture in real-time using dual IMU sensors placed on the upper and lower back. The system leverages Bluetooth Low Energy (BLE) communication, sensor fusion, and **entirely local logic** for posture classification — no machine learning or internet connectivity is required.

SmartStance is designed for the **modern working population**, especially **9-to-5 office workers aged 25–45**, who are or will be affected by back pain and poor posture due to prolonged sitting. This system brings proactive posture care directly to the people who need it most — efficiently, affordably, and portably.

---

## 🚀 Why SmartStance is a NOVEL Innovation
- Unlike generic posture correctors or single-sensor devices, SmartStance uses **dual IMUs** for **relative spine curvature tracking**.
- It’s **entirely local-first**, fully functional **without cloud or internet** — a rare feat in modern wearable tech.
- It uses a **bare-metal implementation of nRF51822**, which is both low-cost and rarely explored in this context.
- **No other posture app** combines hardware, offline BLE logic, on-device analytics, graphing, and full UI like this.
- Its target audience — office workers in the 25–45 age group — are often overlooked by hardware-first solutions.
- SmartStance is built to be global: usable **anywhere in the world**, even without a connection.

**SmartStance isn't just another wearable — it's the first of its kind. It’s a powerful, portable, and privacy-respecting revolution in spinal wellness.**

---

## 📐 System Architecture

### Hardware Components
- **IMU Sensors**: Two (Upper back and Lower back) – MPU6050 or L3G4200D.
- **Microcontroller**: nRF51822 (preferred) or ESP32 with BLE support.
- **Power**: Rechargeable Li-ion or Li-Po battery.

### Why nRF51822?
- Chosen for its **ultra-low power BLE capabilities**, **tiny footprint**, and **cost-effectiveness**.
- While most developers default to ESP32, **SmartStance pushes the frontier by leveraging nRF51822**, a chip rarely explored in DIY posture projects.
- **Flashing the firmware** was a non-trivial feat: it required **OpenOCD**, **ST-Link v2**, and manual power cycles due to the absence of a RESET line.
- Very few documented examples exist — **this is a pioneering implementation** that proves nRF51822 can support real-time posture tracking.
- The use of nRF51822 gives SmartStance its unique edge — a **lightweight, low-cost, and scalable hardware platform** ideal for wearable posture tech.

### Data Flow
1. **IMUs** measure orientation (Roll, Pitch, Yaw).
2. **Microcontroller** calculates ΔRPY (Upper - Lower).
3. Sends posture data and battery level in JSON over BLE.
4. Flutter app receives, parses, classifies, and stores data locally.

---

## 🧠 Posture Detection Logic

All logic is implemented inside the **Flutter application** —
no external ML model, cloud processing, or remote server needed.

### Relative RPY Angle Interpretation (Upper - Lower Back)
| Posture              | ΔRoll (X) | ΔPitch (Y)       | ΔYaw (Z)        | Description                                 |
|----------------------|-----------|------------------|-----------------|---------------------------------------------|
| Ideal / Neutral      | ~0°       | ~0° to ±5°       | ~0° to ±5°      | Straight, aligned back                     |
| Slouching Forward    | ~0°       | > +10°           | ~0° to ±5°      | Upper leans forward                        |
| Slouching Backward   | ~0°       | < -10°           | ~0° to ±5°      | Upper leans backward                       |
| Leaning Left         | > +10°    | ~0° to ±5°       | ~0° to ±5°      | Torso tilts left                           |
| Leaning Right        | < -10°    | ~0° to ±5°       | ~0° to ±5°      | Torso tilts right                          |
| Twisting Left        | ~0°       | ~0° to ±5°       | > +10°          | Upper body twisted left                    |
| Twisting Right       | ~0°       | ~0° to ±5°       | < -10°          | Upper body twisted right                   |
| Hunched (Upper curl) | > +10°    | > +10°           | ~0°             | Curved, forward hunch                      |
| Slumped (Lazy S)     | < -10°    | > +10°           | ~0°             | Forward slouch + backward lean             |

### Notes
- JSON values received over BLE: `{ "roll": ΔX, "pitch": ΔY, "yaw": ΔZ, "battery": XX }`
- Classification is done on-device using conditional logic.
- Sign of angle gives direction; magnitude gives severity.
- Low-pass filters help reduce jitter.

### Development
- Machine Learning will allow SmartStance to scale and adapt to a wide variety of users, including those with medical abnormalities like scoliosis or asymmetrical spinal structures.

- Personalized posture models can be trained from user-specific data and fine-tuned for unique body mechanics.

- ML will empower the device to assist doctors and physiotherapists with analytics, comparisons, and trend detection.
 
- It will also enable predictive posture correction, identifying and alerting on patterns before they become health risks.

- SmartStance is not just a monitoring system — it is a preventative care solution. By detecting bad posture habits early and offering real-time feedback, it can help reduce chronic pain, improve ergonomics, and even mitigate long-term spinal damage.

- In clinical settings, the device could become a valuable tool for medical professionals. Doctors can track patient recovery, evaluate therapy progress, and make data-driven treatment decisions using posture history and metrics collected completely offline.

---

## 💻 Flutter Application

### Project Setup
- Framework: **Flutter**
- State Management: **Provider**
- Routing: **GoRouter** for screen transitions
- Local DB: **sqflite** (SQLite)
- BLE Communication: **flutter_blue_plus**
- Charts: **fl_chart**
- Secure Storage: **flutter_secure_storage**
- CSV Export: **csv**

### Screens Overview

#### 1. 🟦 Landing Screen
- Initial loading + Bluetooth and storage checks.
- Navigates based on previous session or auto-connect.

#### 2. 🟩 SignIn / SignUp Screen
- Currently uses local SQLite for user login.
- Firebase Auth planned for future versions.

#### 3. 🟨 Home Screen
- Live posture label + back angle display.
- Real-time updates from BLE stream.
- Animated avatar showing user posture.


#### 4. 🟥 Progress Screen
- Graphs for Today / Past 3hours / This Week.
- Statistics on posture consistency.
- We provide 4 Charts (3 line and 1 pie chart)
- Downloadable report in pdf format which gives the statistics of that day with recommendations and tips from the app.

#### 5. 🟧 Chatbot Screen
- Ask common questions like “What does my current posture mean?” or “How can I correct a slouch?”
- Get personalized tips based on their recent posture history
- Understand the impact of posture on spine health and productivity
- Receive reminders and motivational nudges to sit upright

#### 5 ⚙️ Settings Screen
- Export csv data, auto-connect toggle, CSV tools.

---

## 🔗 BLE Integration

### nRF51822 BLE Configuration
- Custom BLE service with a characteristic UUID.
- Sends JSON every 5 seconds with RPY values and battery %.
- Example: `{ "roll": 12.3, "pitch": 7.1, "yaw": 3.2, "battery": 91 }`

### Flutter BLE Logic
- Uses flutter_blue_plus to scan, connect, and stream data.
- JSON is parsed and inserted into local database.
- Posture classification is performed locally.

---

## 🛢️ SQLite Database Schema

**Users Table**
| Column     | Type    |
|------------|---------|
| id         | TEXT PK |
| email      | TEXT    |
| password   | TEXT    |
| isSignedin | binary  |

**PostureData Table**
| Column     | Type        |
|------------|-------------|
| id         | INTEGER PK  |
| userId     | TEXT FK     |
| timestamp  | TEXT        |
| posture    | TEXT        |
| deltaX     | REAL        |
| deltaY     | REAL        |
| deltaZ     | REAL        |
| battery    | INTEGER     |

---

## 🌍 Offline-First Architecture
- **No internet connection required** to use the app.
- Entire logic runs locally on-device.
- SQLite used to persist data.
- Ideal for use in rural areas, clinics, or homes with limited connectivity.
- Firebase Auth & cloud sync are planned for future versions.

---

## 🧑‍💻 Developer Info
**Rohan Joshi**  
App Developer + Software&Hardware Integration 

**Adi Khera**
Embedded Systems Developer + Product designer

**Dikshant Ubale**
Embedded Systems Developer + Firmware design

**Omkar Iyer**
Embedded System Developer + PCB Designer + Firmware

---

## 📦 Key Flutter Plugins Used
| Plugin                | Purpose                          |
|----------------------|----------------------------------|
| flutter_blue_plus     | BLE scanning and connection      |
| go_router             | Declarative routing              |
| provider              | State management                 |
| sqflite               | Local SQLite database            |
| flutter_secure_storage| Store tokens/creds securely      |
| fl_chart              | Graph rendering                  |
| csv                   | CSV export                       |

---

## 📬 Contact
- GitHub: [yourgithub]/SmartStance
- Email: dev_smartstance@gmail.com

---

## 📄 License
MIT License – See [LICENSE](./LICENSE) file for full details.
