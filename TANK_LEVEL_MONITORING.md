# Tank Level Monitoring System

## Overview
The Tank Level Monitoring System is a comprehensive Android application feature that provides real-time monitoring of water tank levels using ultrasonic sensors. It includes live graphical visualization, data tracking, and alert notifications.

## Features

### 1. Live Tank Visualization
- **Custom Tank View**: Animated water level display with wave effects
- **Real-time Updates**: Live data from ultrasonic sensors
- **Visual Indicators**: Color-coded level status (Empty, Low, Medium, High, Full)

### 2. Data Monitoring
- **Ultrasonic Sensor Integration**: Reads distance data from ESP32/Blynk
- **Tank Calculations**: Automatic volume and percentage calculations
- **Historical Data**: Chart view showing level trends over time

### 3. Configuration
- **Tank Settings**: Configurable tank dimensions (height, diameter)
- **Sensor Calibration**: Manual sensor calibration feature
- **Data Sources**: Support for Firebase and Blynk data sources

### 4. Alerts & Notifications
- **Low Level Alerts**: Notifications when water level drops below 20%
- **High Level Alerts**: Notifications when tank is nearly full (90%+)
- **Critical Alerts**: Emergency notifications for empty tank

### 5. User Interface
- **Modern Material Design**: Clean, intuitive interface
- **Responsive Layout**: Works on various screen sizes
- **Navigation Integration**: Seamless integration with main app navigation

## Technical Implementation

### Components

#### 1. TankLevelView (Custom View)
```kotlin
class TankLevelView : View {
    // Animated water level visualization
    // Wave effects and gradient fills
    // Real-time level updates
}
```

#### 2. TankLevelFragment
```kotlin
class TankLevelFragment : Fragment {
    // Main UI controller
    // Data binding and user interactions
    // Live monitoring controls
}
```

#### 3. TankLevelViewModel
```kotlin
class TankLevelViewModel : ViewModel {
    // Data management and calculations
    // Sensor data processing
    // Alert system integration
}
```

#### 4. TankLevelChartView
```kotlin
class TankLevelChartView : View {
    // Historical data visualization
    // Real-time chart updates
    // Grid and axis labels
}
```

### Data Flow

1. **Sensor Data**: Ultrasonic sensor → ESP32 → Firebase/Blynk
2. **App Processing**: Repository → ViewModel → Fragment
3. **UI Updates**: Fragment → Custom Views → User Interface
4. **Alerts**: ViewModel → NotificationService → System Notifications

### Key Calculations

#### Water Level Percentage
```kotlin
val waterLevelPercentage = ((tankHeight - sensorDistance) / tankHeight).coerceIn(0f, 1f)
```

#### Water Volume
```kotlin
val volumeLiters = (Math.PI * radius * radius * waterLevel) / 1000
```

#### Level Status
- Empty: 0-5%
- Very Low: 5-20%
- Low: 20-40%
- Medium: 40-70%
- High: 70-90%
- Full: 90-100%

## Usage Instructions

### 1. Accessing Tank Level Monitor
- Open the app
- Navigate to "Tank Level" from the side menu
- View current tank status and live data

### 2. Starting Live Monitoring
- Tap "Start Monitoring" button
- Data updates every 5 seconds
- Real-time visualization updates

### 3. Configuring Tank Settings
- Tap "Tank Settings" button
- Enter tank height and diameter
- Save configuration

### 4. Calibrating Sensor
- Tap "Calibrate" button
- Follow on-screen instructions
- Verify sensor accuracy

### 5. Data Source Selection
- Choose between Firebase and Blynk
- Toggle data source as needed
- Refresh data manually if needed

## Integration with ESP32

### Hardware Requirements
- ESP32 development board
- HC-SR04 ultrasonic sensor
- Power supply and wiring

### ESP32 Code Integration
The system works with existing ESP32 water monitoring code:
- Distance measurement via ultrasonic sensor
- Data transmission to Firebase/Blynk
- Real-time data streaming

### Data Format
Expected data structure:
```json
{
  "timestamp": "2024-01-01T12:00:00Z",
  "distance": 30.5,
  "temperature": 25.0,
  "turbidity": 25.0
}
```

## Alert System

### Notification Types
1. **Low Level Alert**: Water level below 20%
2. **High Level Alert**: Tank nearly full (90%+)
3. **Critical Alert**: Tank completely empty

### Alert Triggers
- Automatic monitoring during active sessions
- Configurable threshold levels
- User-dismissible notifications

## Future Enhancements

### Planned Features
1. **Data Export**: CSV/PDF reports
2. **Multiple Tanks**: Support for multiple tank monitoring
3. **Advanced Analytics**: Trend analysis and predictions
4. **IoT Integration**: Direct sensor communication
5. **Cloud Storage**: Enhanced data persistence

### Technical Improvements
1. **Performance Optimization**: Efficient data handling
2. **Offline Support**: Local data caching
3. **Custom Alerts**: User-defined alert thresholds
4. **Data Visualization**: Advanced charts and graphs

## Troubleshooting

### Common Issues
1. **No Data Display**: Check sensor connection and data source
2. **Incorrect Readings**: Calibrate sensor and verify tank settings
3. **Missing Notifications**: Check notification permissions
4. **Connection Errors**: Verify network connectivity

### Debug Information
- Enable debug logging for detailed information
- Check sensor calibration values
- Verify tank configuration settings
- Monitor data source connectivity

## Support

For technical support or feature requests, please refer to the main project documentation or contact the development team. 