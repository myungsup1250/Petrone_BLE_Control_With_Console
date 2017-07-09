#pragma once
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <bthdef.h>
#include <Bluetoothleapis.h>
#pragma comment(lib, "SetupAPI")
#pragma comment(lib, "BluetoothApis.lib")
#define PETRONE_CUSTOM_SERVICE_UUID "{C320DF00-7891-11E5-8BCF-FEFF819CDC9F}" //we use UUID for an Petrone device 
//#define PETRONE_CUSTOM_SERVICE_UUID "{C320DF01-7891-11E5-8BCF-FEFF819CDC9F}" //we use UUID for an Petrone device 
//#define PETRONE_CUSTOM_SERVICE_UUID "{C320DF02-7891-11E5-8BCF-FEFF819CDC9F}" //we use UUID for an Petrone device 

using namespace std;

#pragma region Variables Region
PBTH_LE_GATT_SERVICE pServiceBuffer, currGattServ;
HANDLE hLEDevice;
HRESULT hr;
PBTH_LE_GATT_CHARACTERISTIC pCharBuffer, currGattChar, pWritableChar;
PBTH_LE_GATT_DESCRIPTOR pDescriptorBuffer, currGattDescriptor;
PBTH_LE_GATT_DESCRIPTOR_VALUE pDescValueBuffer;
#pragma endregion

#pragma region Petrone Enum Class Data Implementation
enum class DataType
{
	None = 0,
	// 시스템 정보
	Ping, // 통신 확인(reserve)
	Ack, // 데이터 수신에 대한 응답
	Error, // 오류(reserve, 비트 플래그는 추후에 지정)
	Request, // 지정한 타입의 데이터 요청
	Passcode, // 새로운 페어링 비밀 번호(페어링 비밀 번호 변경 시 사용)
			  // 조종, 명령
			  Control = 0x10, // 조종
			  Command, // 명령
			  Command2, // 다중 명령(2가지 설정을 동시에 변경)
			  Command3, // 다중 명령(3가지 설정을 동시에 변경)
						// LED
						LedMode = 0x20, // LED 모드 지정
						LedMode2, // LED 모드 2개 지정
						LedModeCommand, // LED 모드, 커맨드
						LedModeCommandIr, // LED 모드, 커맨드, IR 데이터 송신
						LedModeColor, // LED 모드 3색 직접 지정
						LedModeColor2, // LED 모드 3색 직접 지정 2개
						LedEvent, // LED 이벤트
						LedEvent2, // LED 이벤트 2개,
						LedEventCommand, // LED 이벤트, 커맨드
						LedEventCommandIr, // LED 이벤트, 커맨드, IR 데이터 송신
						LedEventColor, // LED 이벤트 3색 직접 지정
						LedEventColor2, // LED 이벤트 3색 직접 지정 2개
						LedModeDefaultColor, // LED 초기 모드 3색 직접 지정
						LedModeDefaultColor2, // LED 초기 모드 3색 직접 지정 2개
											  // 상태
											  Address = 0x30, // IEEE address
											  State, // 드론의 상태(비행 모드, 방위기준, 배터리량)
											  Attitude, // 드론의 자세
											  GyroBias, // 자이로 바이어스 값
											  TrimAll, // 전체 트림
											  TrimFlight, // 비행 트림
											  TrimDrive, // 주행 트림
														 // 데이터 송수신
														 IrMessage = 0x40, // IR 데이터 송수신
														 EndOfType
};
enum class PetroneCommandType
{
	None = 0, // 이벤트 없음
			  // 설정
			  ModePetrone = 0x10, // 페트론 동작 모드 전환
								  // 제어
								  Coordinate = 0x20, // 방위 기준 변경
								  Trim, // 트림 변경
								  FlightEvent, // 비행 이벤트 실행
								  DriveEvent, // 주행 이벤트 실행
								  Stop, // 정지
								  ResetHeading = 0x50, // 방향을 리셋(앱솔루트 모드 일 때 현재 heading을 0도로 변경)
								  ClearGyroBiasAndTrim, // 자이로 바이어스와 트림 설정 초기화
														//통신
														PairingActivate = 0x80, // 페어링 활성화
														PairingDeactivate, // 페어링 비활성화
														TerminateConnection, // 연결 종료
																			 // 요청
																			 Request = 0x90, // 지정한 타입의 데이터 요청
																			 EndOfType
};
enum class ModePetrone
{
	None = 0,
	Flight = 0x10, // 비행 모드(가드 포함)
	FlightNoGuard, // 비행 모드(가드 없음)
	FlightFPV, // 비행 모드(FPV)
	Drive = 0x20, // 주행 모드
	DriveFPV, // 주행 모드(FPV)
	Test = 0x30, // 테스트 모드
	EndOfType
};
enum class Coordinate
{
	None = 0,
	Absolute, // 고정 좌표계
	Relative, // 상대 좌표계
	EndOfType
};
enum class Trim
{
	None = 0,
	RollIncrease, // Roll 증가
	RollDecrease, // Roll 감소
	PitchIncrease, // Pitch 증가
	PitchDecrease, // Pitch 감소
	YawIncrease, // Yaw 증가
	YawDecrease, // Yaw 감소
	ThrottleIncrease, // Throttle 증가
	ThrottleDecrease, // Throttle 감소
	EndOfType
};
enum class FlightEvent
{
	None = 0,
	TakeOff, // 이륙
	FlipFront, // 회전
	FlipRear, // 회전
	FlipLeft, // 회전
	FlipRight, // 회전
	Stop, // 정지
	Landing, // 착륙
	Reverse, // 뒤집기
	Shot, // 미사일을 쏠 때 움직임
	UnderAttack, // 미사일을 맞을 때 움직임
	Square, // 정방향 돌기
	CircleLeft, // 왼쪽으로 회전
	CircleRight, // 오른쪽으로 회전
	Rotate180, // 180도 회전
	EndOfType
};
enum class Request
{
	// 상태
	Address = 0x30, // IEEE address
	State, // 드론의 상태(비행 모드, 방위기준, 배터리량)
	Attitude, // 드론의 자세(Vector)
	GyroBias, // 자이로 바이어스 값(Vector)
	TrimAll, // 전체 트림
	TrimFlight, // 비행 트림
	TrimDrive, // 주행 트림
};
enum class LEDMode
{
	None = 0,
	EyeNone = 0x10,
	EyeHold, // 지정한 색상을 계속 켬
	EyeMix, // 순차적으로 LED 색 변경
	EyeFlicker, // 깜빡임
	EyeFlickerDouble, // 깜빡임(두 번 깜빡이고 깜빡인 시간만큼 꺼짐)
	EyeDimming, // 밝기 제어하여 천천히 깜빡임
	ArmNone = 0x40,
	ArmHold, // 지정한 색상을 계속 켬
	ArmMix, // 순차적으로 LED 색 변경
	ArmFlicker, // 깜빡임
	ArmFlickerDouble, // 깜빡임(두 번 깜빡이고 깜빡인 시간만큼 꺼짐)
	ArmDimming, // 밝기 제어하여 천천히 깜빡임
	ArmFlow, // 앞에서 뒤로 흐름
	ArmFlowReverse, // 뒤에서 앞으로 흐름
	EndOfType
};
enum class Colors
{
	AliceBlue, AntiqueWhite, Aqua,
	Aquamarine, Azure, Beige,
	Bisque, Black, BlanchedAlmond,
	Blue, BlueViolet, Brown,
	BurlyWood, CadetBlue, Chartreuse,
	Chocolate, Coral, CornflowerBlue,
	Cornsilk, Crimson, Cyan,
	DarkBlue, DarkCyan, DarkGoldenRod,
	DarkGray, DarkGreen, DarkKhaki,
	DarkMagenta, DarkOliveGreen, DarkOrange,
	DarkOrchid, DarkRed, DarkSalmon,
	DarkSeaGreen, DarkSlateBlue, DarkSlateGray,
	DarkTurquoise, DarkViolet, DeepPink,
	DeepSkyBlue, DimGray, DodgerBlue,
	FireBrick, FloralWhite, ForestGreen,
	Fuchsia, Gainsboro, GhostWhite,
	Gold, GoldenRod, Gray,
	Green, GreenYellow, HoneyDew,
	HotPink, IndianRed, Indigo,
	Ivory, Khaki, Lavender,
	LavenderBlush, LawnGreen, LemonChiffon,
	LightBlue, LightCoral, LightCyan,
	LightGoldenRodYellow, LightGray, LightGreen,
	LightPink, LightSalmon, LightSeaGreen,
	LightSkyBlue, LightSlateGray, LightSteelBlue,
	LightYellow, Lime, LimeGreen,
	Linen, Magenta, Maroon,
	MediumAquaMarine, MediumBlue, MediumOrchid,
	MediumPurple, MediumSeaGreen, MediumSlateBlue,
	MediumSpringGreen, MediumTurquoise, MediumVioletRed,
	MidnightBlue, MintCream, MistyRose,
	Moccasin, NavajoWhite, Navy,
	OldLace, Olive, OliveDrab,
	Orange, OrangeRed, Orchid,
	PaleGoldenRod, PaleGreen, PaleTurquoise,
	PaleVioletRed, PapayaWhip, PeachPuff,
	Peru, Pink, Plum,
	PowderBlue, Purple, RebeccaPurple,
	Red, RosyBrown, RoyalBlue,
	SaddleBrown, Salmon, SandyBrown,
	SeaGreen, SeaShell, Sienna,
	Silver, SkyBlue, SlateBlue,
	SlateGray, Snow, SpringGreen,
	SteelBlue, Tan, Teal,
	Thistle, Tomato, Turquoise,
	Violet, Wheat, White,
	WhiteSmoke, Yellow, YellowGreen,
	EndOfType
};
enum class ModeFlight
{
	None = 0,
	Ready, // 비행 준비
	TakeOff, // 이륙 (Flight로 자동전환)
	Flight, // 비행
	Flip, // 회전
	Stop, // 강제 정지
	Landing, // 착륙
	Reverse, // 뒤집기
	Accident, // 사고 (Ready로 자동전환)
	Error, // 오류
	EndOfType
};
enum class ModeDrive
{
	None = 0,
	Ready, // 준비
	Start, // 출발
	Drive, // 주행Stop, // 강제 정지
	Accident, // 사고 (Ready로 자동전환)
	Error, // 오류
	EndOfType
};
enum class SensorOrientation
{
	None = 0,
	Normal, // 정상
	ReverseStart, // 뒤집히기 시작
	Reverse, // 뒤집힘
	EndOfType
};
#pragma endregion

#pragma region Command Sample
UCHAR *ArmCyan = new UCHAR[4]{ (UCHAR)DataType::LedMode, (UCHAR)LEDMode::ArmDimming, (UCHAR)Colors::Cyan, (UCHAR)0x05 };
UCHAR *EyeCyan = new UCHAR[4]{ (UCHAR)DataType::LedMode, (UCHAR)LEDMode::EyeDimming, (UCHAR)Colors::Cyan, (UCHAR)0x05 };
UCHAR *ArmYellow = new UCHAR[4]{ (UCHAR)DataType::LedMode, (UCHAR)LEDMode::ArmDimming, (UCHAR)Colors::Yellow, (UCHAR)0x05 };
UCHAR *EyeYellow = new UCHAR[4]{ (UCHAR)DataType::LedMode, (UCHAR)LEDMode::EyeDimming, (UCHAR)Colors::Yellow, (UCHAR)0x05 };
UCHAR *ArmRed = new UCHAR[4]{ (UCHAR)DataType::LedMode, (UCHAR)LEDMode::ArmDimming, (UCHAR)Colors::Red, (UCHAR)0x05 };
UCHAR *EyeRed = new UCHAR[4]{ (UCHAR)DataType::LedMode, (UCHAR)LEDMode::EyeDimming, (UCHAR)Colors::Red, (UCHAR)0x05 };
UCHAR *TakeOff = new UCHAR[3]{ (UCHAR)DataType::Command, (UCHAR)PetroneCommandType::FlightEvent, (UCHAR)FlightEvent::TakeOff };
UCHAR *Landing = new UCHAR[3]{ (UCHAR)DataType::Command, (UCHAR)PetroneCommandType::FlightEvent, (UCHAR)FlightEvent::Landing };
UCHAR *EmergencyLanding = new UCHAR[3]{ (UCHAR)DataType::Command, (UCHAR)PetroneCommandType::FlightEvent, (UCHAR)FlightEvent::Stop };
UCHAR *Altitude = new UCHAR[3]{ (UCHAR)DataType::Command, (UCHAR)PetroneCommandType::Request, (UCHAR)0x57 };
UCHAR *Battery = new UCHAR[3]{ (UCHAR)DataType::Command, (UCHAR)PetroneCommandType::Request, (UCHAR)0x31 };
UCHAR *GyroBias = new UCHAR[3]{ (UCHAR)DataType::Command, (UCHAR)PetroneCommandType::Request, (UCHAR)0x33 };
UCHAR *Gyro = new UCHAR[3]{ (UCHAR)DataType::Command, (UCHAR)PetroneCommandType::Request, (UCHAR)0x50 };
UCHAR *Pressure = new UCHAR[3]{ (UCHAR)DataType::Command, (UCHAR)PetroneCommandType::Request, (UCHAR)0x51 };
UCHAR *OpticalFlow = new UCHAR[3]{ (UCHAR)DataType::Command, (UCHAR)PetroneCommandType::Request, (UCHAR)0x52 };
UCHAR *Battery2 = new UCHAR[3]{ (UCHAR)DataType::Command, (UCHAR)PetroneCommandType::Request, (UCHAR)0x54 };
#pragma endregion

void delay(clock_t n);
int8_t S8BitConvert(UCHAR * src, int index);	// Converts bit for Signed 8bit data
int16_t S16BitConvert(UCHAR * src, int index);	// Converts bit for Signed 16bit data
int32_t S32BitConvert(UCHAR * src, int index);	// Converts bit for Signed 32bit data
uint8_t U8BitConvert(UCHAR * src, int index);	// Converts bit for Unsigned 8bit data
uint16_t U16BitConvert(UCHAR * src, int index);	// Converts bit for Unsigned 16bit data
uint32_t U32BitConvert(UCHAR * src, int index);	// Converts bit for Unsigned 32bit data
void Event_Handler(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context);
HANDLE GetBLEHandle(__in GUID AGuid);
void ScanPetrone();
void Send_Command(UCHAR *cmd);
int main();
