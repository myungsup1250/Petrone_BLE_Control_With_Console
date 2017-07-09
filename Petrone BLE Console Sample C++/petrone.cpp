#include "petrone.h"
using namespace std;

//////////////////////////////////////////////////
//       BLE Connection Sample (Petrone)        //
//////////////////////////////////////////////////

void delay(clock_t n) {
	clock_t start = clock();
	while (clock() - start < n);
}

int8_t S8BitConvert(UCHAR * src, int index) {	// Converts bit for Signed 8bit data
	int8_t result = 0;
	if (src[index] >= 128) { // Negative Number
		result = -(~src[index] + 1);
	}
	else {
		result = src[index];
	}
	return result;
}

int16_t S16BitConvert(UCHAR * src, int index) {	// Converts bit for Signed 16bit data
	int16_t result = 0;
	if (src[index + 1] >= 128) { // Negative Number
		result = -(~(src[index] + (src[index + 1] * 256)) + 1);
	}
	else {
		result = src[index] + src[index + 1] * 256;
	}
	return result;
}

int32_t S32BitConvert(UCHAR * src, int index) {	// Converts bit for Signed 32bit data
	int32_t result = 0;
	if (src[index + 3] >= 128) { // Negative Number
		result = -(~(src[index] + (src[index + 1] << 8) + (src[index + 2] << 16) + (src[index + 3] << 24)) + 1);
	}
	else {
		result = src[index] + (src[index + 1] << 8) + (src[index + 2] << 16) + (src[index + 3] << 24);
	}
	return result;
}

uint8_t U8BitConvert(UCHAR * src, int index) {	// Converts bit for Unsigned 8bit data
	return src[index];
}

uint16_t U16BitConvert(UCHAR * src, int index) {	// Converts bit for Unsigned 16bit data
	return (src[index] + src[index + 1] << 8);
}

uint32_t U32BitConvert(UCHAR * src, int index) {	// Converts bit for Unsigned 32bit data
	return (src[index] + (src[index + 1] << 8) + (src[index + 2] << 16) + (src[index + 3] << 24));
}

void Event_Handler(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context)
{
	PBLUETOOTH_GATT_VALUE_CHANGED_EVENT ValueChangedEventParameters = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT)EventOutParameter;
	UCHAR *rcvData = ValueChangedEventParameters->CharacteristicValue->Data;
	HRESULT hr;
	if (0 == ValueChangedEventParameters->CharacteristicValue->DataSize) {
		hr = E_FAIL;
		cout << "datasize 0" << endl;
	}
	else {
		////cout << "ReceivedDATA : ";	// Prints the received data
		//for (int i = 0; i<(int)(ValueChangedEventParameters->CharacteristicValue->DataSize); i++) {
		//   //cout << (int)(rcvData[i]) << " ";
		//}
		//cout << endl;

		if (rcvData[0] == 0x02) {// ackTime
			int acktime = U16BitConvert(rcvData, 1);
			cout << "acktime : " <<acktime << endl;
		}
		if (rcvData[0] == 0x50) { // IMU Sensor
			int accX = S16BitConvert(rcvData, 1);
			int accY = S16BitConvert(rcvData, 3);
			int accZ = S16BitConvert(rcvData, 5);
			int gyroRoll = S16BitConvert(rcvData, 7);
			int gyroPitch = S16BitConvert(rcvData, 9);
			int gyroYaw = S16BitConvert(rcvData, 11);
			int angleRoll = S16BitConvert(rcvData, 13);
			int anglePitch = S16BitConvert(rcvData, 15);
			int angleYaw = S16BitConvert(rcvData, 17);

			/*cout << "[ accX : " << accX << " | accY : " << accY << " | accZ : " << accZ << " ]" << endl;
			cout << "[ gyroRoll : " << gyroRoll << " | gyroPitch : " << gyroPitch << " | gyroYaw : " << gyroYaw << " ]" << endl;
			cout << "[ angleRoll : " << angleRoll << " | anglePitch : " << anglePitch << " | angleYaw : " << angleYaw << " ]" << endl;*/

		}
		if (rcvData[0] == 0x57) {// Range
			int altitude = U16BitConvert(rcvData, 11);
			//cout << altitude << endl;
		}
	}
}

HANDLE GetBLEHandle(__in GUID AGuid)
{
	HDEVINFO hDI;
	SP_DEVICE_INTERFACE_DATA did;
	SP_DEVINFO_DATA dd;
	GUID BluetoothInterfaceGUID = AGuid;
	HANDLE hComm = NULL;

	hDI = SetupDiGetClassDevs(&BluetoothInterfaceGUID, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if (hDI == INVALID_HANDLE_VALUE)
		return NULL;

	did.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	dd.cbSize = sizeof(SP_DEVINFO_DATA);

	for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDI, NULL, &BluetoothInterfaceGUID, i, &did); i++) {
		SP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;
		DeviceInterfaceDetailData.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		DWORD size = 0;

		if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, NULL, 0, &size, 0)) {
			int err = GetLastError();

			if (err == ERROR_NO_MORE_ITEMS) {
				cout << "ERROR NO MORE ITEMS" << endl;
				break;
			}

			PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)GlobalAlloc(GPTR, size);

			pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, pInterfaceDetailData, size, &size, &dd)) {
				cout << "break" << endl;
				break;
			}

			hComm = CreateFile(pInterfaceDetailData->DevicePath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

			GlobalFree(pInterfaceDetailData);
		}
	}

	SetupDiDestroyDeviceInfoList(hDI);
	return hComm;
}

void ScanPetrone() {
	//Step 1: find the BLE device handle from its GUID
	GUID AGuid;
	CLSIDFromString(TEXT(PETRONE_CUSTOM_SERVICE_UUID), &AGuid);

	//now get the handle
	hLEDevice = GetBLEHandle(AGuid);

	//Step 2: Get a list of services that the device advertises
	// first send 0,NULL as the parameters to BluetoothGATTServices inorder to get the number of
	// services in serviceBufferCount
	USHORT serviceBufferCount;
	////////////////////////////////////////////////////////////////////////////
	// Determine Services Buffer Size
	////////////////////////////////////////////////////////////////////////////

	hr = BluetoothGATTGetServices(hLEDevice, 0, NULL, &serviceBufferCount, BLUETOOTH_GATT_FLAG_NONE);

	if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr)
		cout << "BluetoothGATTGetServices - Buffer Size " << hr << endl;

	pServiceBuffer = (PBTH_LE_GATT_SERVICE)malloc(sizeof(BTH_LE_GATT_SERVICE) * serviceBufferCount);

	if (pServiceBuffer == NULL)
		cout << "pServiceBuffer is out of memory" << endl;
	else
		RtlZeroMemory(pServiceBuffer, sizeof(BTH_LE_GATT_SERVICE) * serviceBufferCount);

	////////////////////////////////////////////////////////////////////////////
	// Retrieve Services
	////////////////////////////////////////////////////////////////////////////
	USHORT numServices;
	hr = BluetoothGATTGetServices(hLEDevice, serviceBufferCount, pServiceBuffer, &numServices, BLUETOOTH_GATT_FLAG_NONE);

	if (S_OK != hr)
		cout << "BluetoothGATTGetServices - Buffer Size %d" << hr << endl;
	if (serviceBufferCount != numServices)
		cout << "buffer size and buffer size actual size mismatch" << endl;

	//Step 3: now get the list of charactersitics. note how the pServiceBuffer is required from step 2
	////////////////////////////////////////////////////////////////////////////
	// Determine Characteristic Buffer Size
	////////////////////////////////////////////////////////////////////////////
	USHORT charBufferSize;
	hr = BluetoothGATTGetCharacteristics(hLEDevice, currGattServ, 0, NULL, &charBufferSize, BLUETOOTH_GATT_FLAG_NONE);

	if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr)
		cout << "BluetoothGATTGetCharacteristics - Buffer Size " << hr << endl;

	if (charBufferSize > 0) {
		pCharBuffer = (PBTH_LE_GATT_CHARACTERISTIC)malloc(charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));

		if (pCharBuffer != NULL)
			RtlZeroMemory(pCharBuffer, charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));
		else
			cout << "pCharBuffer is out of memory" << endl;

		////////////////////////////////////////////////////////////////////////////
		// Retrieve Characteristics
		////////////////////////////////////////////////////////////////////////////
		USHORT numChars;
		hr = BluetoothGATTGetCharacteristics(hLEDevice, currGattServ, charBufferSize, pCharBuffer, &numChars, BLUETOOTH_GATT_FLAG_NONE);

		if (S_OK != hr)
			cout << "BluetoothGATTGetCharacteristics - Actual Data " << hr << endl;
		if (charBufferSize != numChars)
			cout << "buffer size and actual buffer size mismatch" << endl;
	}

	//Step 4: now get the list of descriptors. note how the pCharBuffer is required from step 3
	//descriptors are required as we descriptors that are notification based will have to be written
	//once IsSubcribeToNotification set to true, we set the appropriate callback function
	//need for setting descriptors for notification according to
	//http://social.msdn.microsoft.com/Forums/en-US/11d3a7ce-182b-4190-bf9d-64fefc3328d9/windows-bluetooth-le-apis-event-callbacks?forum=wdk
	for (int i = 0; i < charBufferSize; i++) {
		currGattChar = &pCharBuffer[i];
		///////////////////////////////////////////////////////////////////////////
		// Determine Descriptor Buffer Size
		////////////////////////////////////////////////////////////////////////////
		USHORT descriptorBufferSize;
		hr = BluetoothGATTGetDescriptors(hLEDevice, currGattChar, 0, NULL, &descriptorBufferSize, BLUETOOTH_GATT_FLAG_NONE);

		if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr)
			cout << "BluetoothGATTGetDescriptors - Buffer Size " << hr << endl;

		if (descriptorBufferSize > 0) {
			pDescriptorBuffer = (PBTH_LE_GATT_DESCRIPTOR)malloc(descriptorBufferSize * sizeof(BTH_LE_GATT_DESCRIPTOR));

			if (pDescriptorBuffer != NULL)
				RtlZeroMemory(pDescriptorBuffer, descriptorBufferSize);
			else
				cout << "pDescriptorBuffer is out of memory" << endl;

			////////////////////////////////////////////////////////////////////////////
			// Retrieve Descriptors
			////////////////////////////////////////////////////////////////////////////
			USHORT numDescriptors;
			hr = BluetoothGATTGetDescriptors(hLEDevice, currGattChar, descriptorBufferSize, pDescriptorBuffer, &numDescriptors, BLUETOOTH_GATT_FLAG_NONE);

			if (S_OK != hr)
				cout << "BluetoothGATTGetDescriptors - Actual Data " << hr << endl;

			if (descriptorBufferSize != numDescriptors)
				cout << "buffer size and actual buffer size mismatch" << endl;

			for (int j = 0; j < numDescriptors; j++) {
				currGattDescriptor = &pDescriptorBuffer[j];

				////////////////////////////////////////////////////////////////////////////
				// Determine Descriptor Value Buffer Size
				////////////////////////////////////////////////////////////////////////////
				USHORT descValueDataSize;
				hr = BluetoothGATTGetDescriptorValue(hLEDevice, currGattDescriptor, 0, NULL, &descValueDataSize, BLUETOOTH_GATT_FLAG_NONE);

				if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr)
					cout << "BluetoothGATTGetDescriptorValue - Buffer Size " << hr << endl;

				pDescValueBuffer = (PBTH_LE_GATT_DESCRIPTOR_VALUE)malloc(descValueDataSize);

				if (pDescValueBuffer != NULL)
					RtlZeroMemory(pDescValueBuffer, descValueDataSize);
				else
					cout << "pDescValueBuffer out of memory" << endl;

				////////////////////////////////////////////////////////////////////////////
				// Retrieve the Descriptor Value
				////////////////////////////////////////////////////////////////////////////
				USHORT descValueSizeRequired;
				hr = BluetoothGATTGetDescriptorValue(hLEDevice, currGattDescriptor, (ULONG)descValueDataSize, pDescValueBuffer, &descValueSizeRequired, BLUETOOTH_GATT_FLAG_NONE);

				if (S_OK != hr)
					cout << "BluetoothGATTGetDescriptorValue - Actual Data " << hr << endl;
				if (descValueDataSize != descValueSizeRequired)
					cout << "buffer size and actual buffer size mismatch" << endl;

				//you may also get a descriptor that is read (and not notify) and i am guessing the attribute handle is out of limits
				// we set all descriptors that are notifiable to notify us via IsSubstcibeToNotification
				if (currGattDescriptor->AttributeHandle < 255 && currGattDescriptor->DescriptorType == 2) {
					cout << "ClientCharacteristicConfiguration" << endl;
					BTH_LE_GATT_DESCRIPTOR_VALUE newValue;
					RtlZeroMemory(&newValue, sizeof(newValue));

					newValue.DescriptorType = ClientCharacteristicConfiguration;
					newValue.ClientCharacteristicConfiguration.IsSubscribeToNotification = TRUE;
					hr = BluetoothGATTSetDescriptorValue(hLEDevice, currGattDescriptor, &newValue, BLUETOOTH_GATT_FLAG_NONE);

					if (S_OK != hr)
						cout << "BluetoothGATTGetDescriptorValue - Actual Data %d" << hr << endl;
					else
						cout << "Setting Notification for Serivice handle " << currGattDescriptor->ServiceHandle << endl;
				}
			}
		}
		//set the appropriate callback function when the descriptor change value
		BLUETOOTH_GATT_EVENT_HANDLE EventHandle;
		if (currGattChar->IsNotifiable) {
			BTH_LE_GATT_EVENT_TYPE EventType = CharacteristicValueChangedEvent;
			BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION EventParameterIn;
			EventParameterIn.Characteristics[0] = *currGattChar;
			EventParameterIn.NumCharacteristics = 1;
			hr = BluetoothGATTRegisterEvent(hLEDevice, EventType, &EventParameterIn, (PFNBLUETOOTH_GATT_EVENT_CALLBACK)Event_Handler, NULL, &EventHandle, BLUETOOTH_GATT_FLAG_NONE);

			if (S_OK != hr)
				cout << "BluetoothGATTRegisterEvent - Actual Data " << hr << endl;
			else
				cout << "Setting Notification for ServiceHandle " << currGattChar->ServiceHandle << endl;
		}
		if (currGattChar->IsWritable || currGattChar->IsWritableWithoutResponse) {	// Keep the Writable GattCharacteristic
			pWritableChar = currGattChar;
		}
	}

	cout << "BLE Connection Completed." << endl << "Start the program? (Y / N) ";
	char answer;
	cin >> answer;
	if (answer == 'Y' || answer == 'y' || answer == 'YES' || answer == 'Yes' || answer == 'yes') {
		cout << "Okay, the program will start in 3 sec!" << endl;	// the User Wants the program to continue
		delay(3000);
	}
	else if (answer == 'N' || answer == 'n' || answer == 'NO' || answer == 'No' || answer == 'no') {
		cout << "Okay, the program will terminate. Good Bye!" << endl;
		exit(0);	// the User terminates the program
	}
}

void Send_Command(UCHAR *cmd) {
	// Calls to BluetoothGATTSetCharacteristicValue
	int length = _msize(cmd);
	PBTH_LE_GATT_CHARACTERISTIC_VALUE newValue = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)malloc(sizeof(BTH_LE_GATT_CHARACTERISTIC_VALUE) + length);

	if (NULL == newValue)
		printf("pCharValueBuffer out of memory\r\n");
	else
		RtlZeroMemory(newValue, sizeof(BTH_LE_GATT_CHARACTERISTIC_VALUE) + length);

	newValue->DataSize = (ULONG)length;
	memcpy(newValue->Data, cmd, length);

	// Set the new characteristic value
	hr = BluetoothGATTSetCharacteristicValue(hLEDevice, pWritableChar, newValue, NULL, BLUETOOTH_GATT_FLAG_WRITE_WITHOUT_RESPONSE);

	if (S_OK != hr)
		cout << "Failed to BluetoothGATTSetCharacteristicValue" << endl;
	if (E_BLUETOOTH_ATT_INSUFFICIENT_RESOURCES == hr)
		cout << "E_BLUETOOTH_ATT_INSUFFICIENT_RESOURCES" << endl;

	free(newValue);
	newValue = NULL;
}

int main() {
	ScanPetrone();
	int count = 0;

	while (1) {	// Sends Color Change Commands
		Sleep(1000);
		//printf("look for notification\n");
		if (count % 3 == 0) {
			cout << "Cyan" << endl;
			Send_Command(ArmCyan);
			Send_Command(EyeCyan);
		}
		else if (count % 3 == 1) {
			cout << "Yellow" << endl;
			Send_Command(ArmYellow);
			Send_Command(EyeYellow);
		}
		else {
			cout << "Red" << endl;
			Send_Command(ArmRed);
			Send_Command(EyeRed);
		}
		count++;
	}

	CloseHandle(hLEDevice);

	if (GetLastError() != NO_ERROR && GetLastError() != ERROR_NO_MORE_ITEMS)
	{
		// Insert error handling here.
		return 1;
	}

	return 0;
}