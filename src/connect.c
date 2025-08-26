#include "../libs/deckify.h"

int initializeBluetooth() {
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
		fprintf(stderr, "Failed to initialize Winsock: %d\n", WSAGetLastError());
		return -1;
	}
	return 0;
}

int discoverDeviceByName(const wchar_t *device_name, BLUETOOTH_ADDRESS *out_addr) {
	BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = {0};
	searchParams.dwSize = sizeof(searchParams);
	searchParams.fReturnAuthenticated = TRUE;
	searchParams.fReturnRemembered = TRUE;
	searchParams.fReturnConnected = TRUE;
	searchParams.fReturnUnknown = TRUE;
	searchParams.fIssueInquiry = TRUE;
	searchParams.cTimeoutMultiplier = 2;

	BLUETOOTH_DEVICE_INFO deviceInfo = {0};
	deviceInfo.dwSize = sizeof(deviceInfo);

	HBLUETOOTH_DEVICE_FIND hFind = BluetoothFindFirstDevice(&searchParams, &deviceInfo);
	if (hFind == NULL) {
		printf("No Bluetooth devices found\n");
		return -3;
	}

	do {
		if (wcscmp(deviceInfo.szName, device_name) == 0) {
			*out_addr = deviceInfo.Address;
			BluetoothFindDeviceClose(hFind);
			return 0;
		}
	} while (BluetoothFindNextDevice(hFind, &deviceInfo));

	BluetoothFindDeviceClose(hFind);
	return -3;
}

SOCKET connectToDeviceWithUUID(BLUETOOTH_ADDRESS btAddr, GUID service_uuid) {
	SOCKET s = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if (s == INVALID_SOCKET) return INVALID_SOCKET;

	SOCKADDR_BTH sab = {0};
	sab.addressFamily = AF_BTH;
	sab.btAddr = btAddr.ullLong;
	sab.serviceClassId = service_uuid;
	sab.port = 0;

	if (connect(s, (SOCKADDR*)&sab, sizeof(sab)) != 0) {
		int err = WSAGetLastError();
		closesocket(s);
		return INVALID_SOCKET;
	}

	printf("Connected to device at %016llX\n", btAddr.ullLong);

	return s;
}

int startBluetoothListener(ListenerParams *params) {
	if (params == NULL || params->socket == INVALID_SOCKET || params->data_callback == NULL) {
		return -1;
	}

	char buffer[1024];
	int running = 1;

	printf("Starting Bluetooth listener... (Press Ctrl+C to stop)\n");

	while (running) {
		int bytes_received = recv(params->socket, buffer, sizeof(buffer) - 1, 0);
		if (bytes_received == SOCKET_ERROR) {
			int error = WSAGetLastError();
			printf("recv() error: %d\n", error);
			break;
		}
		
		if (bytes_received == 0) {
			printf("Bluetooth connection closed\n");
			break;
		}

		buffer[bytes_received] = '\0';
		
		params->data_callback(buffer, bytes_received);
	}
	free(params);
	return 0;
}

int connectBluetooth(const wchar_t *name, const char *uuidString, SOCKET *socket, void (*handleData)(const char*, int)) {
	if (initializeBluetooth() != 0) {
		printf("Failed to initialize Bluetooth\n");
		return -1;
	}

	BLUETOOTH_ADDRESS addr;
	printf("Searching for device...\n");

	int result = discoverDeviceByName(name, &addr);
	if (result == 0) {
		printf("Device found! Address: %012llX\n", addr.ullLong);
		
		connect:
		GUID service_uuid;
		RPC_STATUS status = UuidFromStringA("248dd985-12af-420b-b46b-fd76109a64e9", &service_uuid);
		if (status != RPC_S_OK) {
			fprintf(stderr, "Failed to parse UUID: %d\n", status);
			return -1;
		}
		*socket = connectToDeviceWithUUID(addr, service_uuid);
		if (*socket != INVALID_SOCKET) {
			printf("Connected to device successfully.\n");
			pthread_t listener_thread;
			ListenerParams *params = malloc(sizeof(ListenerParams));
			if (params == NULL) {
				printf("Failed to allocate memory for listener parameters.\n");
				cleanupBluetooth(*socket);
				return -1;
			}
			params->socket = *socket;
			params->data_callback = handleData;
			if (pthread_create(&listener_thread, NULL, (void* (*)(void*))startBluetoothListener, (void*)params) != 0) {
				printf("Failed to create Bluetooth listener thread.\n");
				cleanupBluetooth(*socket);
				return -1;
			}
			pthread_detach(listener_thread);
		} else {
			printf("Failed to connect to device. Retrying...\n");
			Sleep(100);
			goto connect;
		}
	} else {
		printf("Device not found. Error code: %d\n", result);
	}
	return 0;
}

void cleanupBluetooth(SOCKET s) {
	closesocket(s);
	WSACleanup();
}