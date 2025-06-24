# Plant-It — Smart Watering System 🌿🚰

**Plant-It** is a fully automated plant watering system built on an ESP8266 microcontroller. It identifies plants using Pl@ntNet and OpenAI, retrieves their optimal moisture ranges, and waters them accordingly — all while connecting live to a React web dashboard for easy monitoring.

---

📄 Project Documentation (PDF)  
🎥 Demo Video

---

## 🌱 Background

There are many plants in my house. Every corner you turn, in every room, there is a plant. Thanks to my dad, an avid gardener, my house has become a thriving extension of my backyard garden. However, many plants in the house go without water, far longer than they should. Due to the sheer number of them, it is easy to lose track of the ones that need water. This gave me an idea:  
**What if each plant had its own water supply, which poured just the right amount of water it required?**

---

## 🧠 How It Works

Plant-It automatically identifies your plant, retrieves watering needs, and waters based on live soil readings:

1. **Plant Identification**  
   - Snap a photo of the plant  
   - Use **Pl@ntNet API** to detect species  

2. **Optimal Moisture Retrieval**  
   - Feed species data to **OpenAI API**  
   - Retrieve ideal min/max/optimal soil moisture %  

3. **Microcontroller Logic**  
   - ESP8266 connects via WiFi to a web app  
   - Receives min/max/optimal VWC thresholds (once per session)  
   - Reads soil moisture from sensor  
   - Controls water pump via relay, watering only as needed  
   - Displays live status on I2C LCD  

4. **Web Dashboard**  
   - Built with **React**  
   - Uses **Google Accounts** for individual sign-in  
   - Per-user dashboard to view all plants  
   - See each plant’s watering status, species, family, and care tips  
   - ESP8266 can be paired to any plant at any time — easily swap devices between plants

---

## 🔧 Tech Stack

- **ESP8266** microcontroller (NodeMCU)  
- **Arduino IDE**  
- LiquidCrystal_I2C (LCD)  
- Soil moisture sensor (capacitive)  
- 5V submersible water pump + relay module  
- Web server with REST endpoints  
- **React.js** web app  
- **Google OAuth** for login  
- Pl@ntNet API  
- OpenAI API

---

## ⚡️ Electronics Used

1. **8-channel relay module**  
2. **5V / 3.3V breadboard power supply**  
3. **Potentiometer** (to tune pump flow rate)  
4. **LCM1602 I2C LCD display**  
5. **Capacitive soil moisture sensor v1.2**  
6. **3–6V submersible water pump**  
7. **ESP8266 (NodeMCU)** microcontroller

---

## 🚀 Quickstart

```bash
git clone https://github.com/yashp1932/Plant-It-Automatic-Watering-System.git
cd Plant-It-Automatic-Watering-System
# Open plant_watering_system.ino in Arduino IDE
# Upload to ESP8266 (NodeMCU)
# Power the board and watch it water! 🌱
