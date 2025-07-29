# 🔧 Dual-Mode Motion Tracking IoT System using ESP32 & MPU9250

A real-time motion tracking system powered by an ESP32 and MPU9250 9-DOF sensor, designed to operate seamlessly in both **online** (cloud-connected) and **offline** (fallback AP-hosted) modes. Built with robust real-time data streaming, smart mode switching, and efficient storage, this project is ideal for low-power wearable or embedded IoT deployments.

---

## Features

✅ **Dual-Mode Operation**  
- **Online Mode**: Sends sensor data to **Firebase Realtime Database** over Wi-Fi.  
- **Offline Mode**: Creates a fallback **Access Point (AP)** with a local **WebSocket-based dashboard**.

✅ **Real-Time Streaming**  
- Online: Firebase updates for remote access  
- Offline: Local WebSocket updates for low-latency performance

✅ **Data Storage Optimization**  
- **Batching & Throttling** to reduce Firebase writes  
- **Rolling Log Buffer**: Only latest 1000 entries retained in Firebase  
- **Offline Logging**: Data stored in **SPIFFS** and synced to Firebase on reconnect

✅ **Auto & Manual Fallback Switching**  
- Automatic detection of internet loss  
- Manual override supported

✅ **Sensor Calibration**  
- Offset calibration for accelerometer & gyroscope using user-prompted stillness phase

✅ **Future-Ready**  
- Google Sheets export (via Firebase)  
- Firebase-hosted dashboard (via Vercel)
