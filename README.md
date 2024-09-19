# ESP32 P1 Forwarder

This project polls the API of a Homewizard P1 meter to retrieve energy and gas usage data from your electric meter. The data is then uploaded to a Supabase database for storage and further analysis. 

## Features

- **Data Retrieval**: Connects to a Homewizard P1 meter to obtain energy and gas consumption data.
- **Data Upload**: Sends the retrieved data to a Supabase database, allowing for centralized data storage.
- **ESP32 Support**: Designed to run on an ESP32 microcontroller, making it a cost-effective solution for real-time monitoring.
- **SSL Support**: Includes necessary certificates for secure communication with the Supabase API.

## Prerequisites

- A Homewizard P1 meter.
- An ESP32 microcontroller with Wi-Fi capability.
- A Supabase account with a configured database to store energy data.
- PlatformIO installed on your development environment (for building and uploading the firmware).

## Setup Instructions

1. **Clone the Repository**
   ```bash
   git clone <repository-url>
   cd esp32-p1-forwarder
   ```

2. **Install Dependencies**
   Ensure PlatformIO is installed in your development environment. Refer to the [PlatformIO installation guide](https://platformio.org/install) if needed.

3. **Configure Secrets**
   - Copy the example secrets file:
     ```bash
     cp include/secrets.h.example include/secrets.h
     ```
   - Open `include/secrets.h` and fill in your Wi-Fi credentials and Supabase API keys.

4. **Build and Upload Firmware**
   - Connect your ESP32 to the computer.
   - Use PlatformIO to build and upload the firmware:
     ```bash
     platformio run --target upload
     ```

## Usage

Once the ESP32 is running the firmware, it will:
1. Poll the Homewizard P1 meter's API at regular intervals to obtain energy and gas data.
2. Upload the data to the configured Supabase database.

## Contributing

Feel free to open issues or submit pull requests for any improvements or bug fixes.

## License

This project is licensed under the MIT License.
