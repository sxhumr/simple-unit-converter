# Qt Unit Converter

## Overview
A desktop application developed using Qt and C++.  
It provides accurate conversion between commonly used units through a simple graphical interface.  
The project emphasizes correctness, structure, and maintainable design.

## Functionality
- **Length conversion**
  - Meters to feet
  - Kilometres to miles
- **Weight conversion**
  - Kilograms to pounds
- **Temperature conversion**
  - Celsius to Fahrenheit
- **Currency conversion**
  - Live conversion using current exchange rates

## User Workflow
1. Enter a numeric value  
2. Select source and target units  
3. Submit the conversion  
4. View the result immediately  

## Technical Focus
- Qt Widgets for desktop UI development
- Clear separation between interface and business logic
- Deterministic conversion formulas
- Runtime retrieval of currency exchange rates
- Predictable input and output handling

## Technology Stack
- C++
- Qt 5 or higher
- Qt Creator
- MSVC or MinGW
- CMake when enabled

## Project Structure
src/
main.cpp
converter.cpp
converter.h
ui/
mainwindow.ui
CMakeLists.txt


## Currency Data Handling
- Exchange rates retrieved from a public API
- Rates fetched at runtime
- No hardcoded currency values

## Input Validation and Errors
- Invalid numeric input blocked
- Empty submissions prevented
- Clear feedback presented to the user

## Build Environment
- Developed and tested on Windows
- Qt version defined in project configuration
- Compiler configured through Qt Creator

## How to Run
1. Open the project in Qt Creator  
2. Select a build kit  
3. Build the project  
4. Run the application  

## Planned Enhancements
- Reverse unit conversions
- Expanded unit and currency support
- Additional input validation
- Interface refinements

## License
This project is open source and free to use.


