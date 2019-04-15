/////////////////////////////////////////////////////////////////////// Includes
#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <iostream>

bool TG = false;

HANDLE initialiseComPort();

DWORD WINAPI Receiver(LPVOID lpParam);

void Transmitter(HANDLE handleSetCOM);

//////////////////////////////////////////////////////////// COM port operations
HANDLE initialiseComPort() {

    // Choose port number
    int comPortNumber;

    std::cout << "Enter desired COM port number: ";
    std::cin >> comPortNumber;

    char integer_string[32];

    char chosenPortNum[5] = "COM";

    sprintf(integer_string, "%d", comPortNumber);
    strcat(chosenPortNum, integer_string);

    // Open COM port
    HANDLE handleSetCOM = CreateFile(chosenPortNum,
                                     GENERIC_WRITE | GENERIC_READ,
                                     0,
                                     nullptr,
                                     OPEN_EXISTING,
                                     0,
                                     nullptr);

    constexpr int BYTE_SIZE = 8;
    DCB dcb;
    COMMTIMEOUTS timeCOM;

    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = BYTE_SIZE;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;

    SetCommState(handleSetCOM, &dcb);

    timeCOM.ReadIntervalTimeout = 50;
    timeCOM.ReadTotalTimeoutConstant = 50;
    timeCOM.ReadTotalTimeoutMultiplier = 10;
    timeCOM.WriteTotalTimeoutConstant = 50;
    timeCOM.WriteTotalTimeoutMultiplier = 10;

    SetCommTimeouts(handleSetCOM, &timeCOM);

    if (GetCommState(handleSetCOM, &dcb)) {
        printf("COM port initialization successful \n");
    } else {
        printf("COM port initialization failed\n");
        exit(1);
    }
    return handleSetCOM;
}

DWORD WINAPI Receiver(LPVOID lpParam) {
    HANDLE &portCOM = (HANDLE &) lpParam;
    char sign = 0;
    DWORD getBack = 0;

    while (true) {
        if (!TG) {
            ReadFile(portCOM, &sign, 1, &getBack, NULL);
            if (getBack != 0)
                if (sign != 13)
                    printf("%c", sign);
        } else
            Sleep(10);
    }
}

void Transmitter(HANDLE handleSetCOM) {
    char sign = 0;
    DWORD sent = 0;

    do {
        sign = getch();
        if (sign != 27) {
            if (sign != 13) {
                printf("%c", sign);
                TG = true;
                WriteFile(handleSetCOM, &sign, 1, &sent, nullptr);
                TG = false;
            } else {
                printf("\n");
                TG = true;
                sign = 10;
                WriteFile(handleSetCOM, &sign, 1, &sent, nullptr);
                sign = 13;
                WriteFile(handleSetCOM, &sign, 1, &sent, nullptr);
                TG = false;
            }
        }
        TG = false;
    } while (sign != 27);
}

/////////////////////////////////////////////////////////////////////////// Main
int main() {

    //COM port initialization
    DWORD ID = 0;
    HANDLE handleSetCOM = initialiseComPort();

    HANDLE handleThread = CreateThread(nullptr, 0, Receiver, handleSetCOM, 0, &ID);
    Transmitter(handleSetCOM);

    TerminateThread(handleThread, 0);
    CloseHandle(handleSetCOM);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
