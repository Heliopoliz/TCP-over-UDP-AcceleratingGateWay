Doublegateway : DoubleGateway.o OvertimeResend.o RecvAndSend.o
	g++ -o Doublegateway DoubleGateway.o OvertimeResend.o RecvAndSend.o

DoubleGateway.o : DoubleGateway.cpp stdafx.h RecvAndSend.h
	g++ -c DoubleGateway.cpp

OvertimeResend.o : OvertimeResend.cpp OvertimeResend.h
	g++ -c OvertimeResend.cpp

RecvAndSend.o : RecvAndSend.cpp RecvAndSend.h OvertimeResend.h OvertimeResend.cpp
	g++ -c RecvAndSend.cpp

clean: 
	rm DoubleGateway.o OvertimeResend.o RecvAndSend.o Doublegateway
