# ELEC70126 IoT Coursework
**Student CID:** 06044455  
**Module:** ELEC70126 - Internet of Things and Applications

## Project Overview
A platform named EcoSense is a low-cost IoT system that monitors indoor environmental 
conditions and laptop energy consumption to analyse how temperature, 
humidity, light, and occupancy affect device power consumption.  
**Data collection period:** 21 Feb – 13 Mar 2026 | **Records:** 3,753

## File Description

| File/Folder | Description |
|---|---|
| `Environment Sensor 4in1 Ver4/` | Arduino code for ESP32 sensor node (DHT11, LDR, PIR, DS1307) |
| `DataCollect/` | Raw data collected during 3-week deployment: `BatteryMon/` contains per-session battery logs exported from BatteryMon software; `Battery_cons_combine.xlsx` is the merged battery dataset; `Env_Sensing_Data.xlsx` is the environmental sensor data uploaded from ESP32 |
| `google_app_scripts_dePost.gs.txt` | Google Apps Script for receiving HTTP POST from ESP32 and writing to Google Sheets |
| `data.ipynb` | Data cleaning, correlation analysis and visualisation notebook |
| `df_cleaned.csv` | Final cleaned dataset (3,753 records) |
| `website_dashboard.html` | EcoSense web dashboard front end |
| `data_flow.pptx` | System architecture diagram |
| `Figure/` | All figures used in the report |
| `Screenshot/` | EcoSense web platform screenshots |

## Live Demo
[EcoSense Dashboard](https://peaceandlove521.github.io/ELEC70126-IoT-06044455/IOT_Coursework/website_dashboard.html)
