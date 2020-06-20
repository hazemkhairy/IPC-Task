
#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996) 
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <mutex>

using namespace std;

struct ModiefiedSocket {
	SOCKET sock;
	sockaddr_in address;
};
struct ModiefiedProcess {
	sockaddr_in address;
	int pID;
};
struct Message {
	ModiefiedProcess sender;
	string content;
	string recievedTime;
};
vector <ModiefiedProcess>higher;
vector <ModiefiedProcess>lower;
ModiefiedProcess coordinator;
WSADATA WSAData;
ModiefiedSocket senderSocket;
ModiefiedSocket recieverSocket;
ModiefiedSocket loggerSocket;
int pID;
int recievingPort;

std::mutex cm;


ModiefiedSocket createTCPSocket()
{
	ModiefiedSocket temp;
	temp.sock = socket(AF_INET, SOCK_STREAM, 0);

	temp.address.sin_family = AF_INET;
	temp.address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	temp.address.sin_port = 0;
	if (temp.sock == INVALID_SOCKET)
	{
		cerr << "Can't create socket at process " << pID << endl;
		exit(1);
	}
	int be = bind(temp.sock, (sockaddr*)&temp.address, sizeof(temp.address));

	int length = sizeof(temp.address);
	getsockname(temp.sock, (sockaddr*)&temp.address, &length);

	return temp;
}
ModiefiedSocket createLoggerSocket()
{
	ModiefiedSocket temp;
	temp.sock = socket(AF_INET, SOCK_STREAM, 0);

	temp.address.sin_family = AF_INET;
	temp.address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	temp.address.sin_port = htons(1998);
	if (temp.sock == INVALID_SOCKET)
	{
		cerr << "Can't create socket at process " << pID << endl;
		exit(1);
	}
	return temp;
}

void messageFormater(string message, char* ret)
{
	message = to_string(pID) + '\n' + "127.0.0.1" + "\n" + to_string(recievingPort) + "\n" + message;
	strcpy(ret, message.c_str());
	return;
}
vector <string >split(string toBeSplitted, char delimiter = '\n')
{
	int current, previous = 0;
	current = toBeSplitted.find(delimiter);
	vector<string>ret;
	while (current != std::string::npos) {
		ret.push_back(toBeSplitted.substr(previous, current - previous));
		previous = current + 1;
		current = toBeSplitted.find(delimiter, previous);

	}

	ret.push_back(toBeSplitted.substr(previous, current - previous));
	return ret;
}
void sendToLogger(string m, int sourcePID)
{
	unique_lock<mutex>locker(cm, defer_lock);
	cm.lock();
	time_t now = time(0);
	char* createdTime = ctime(&now);

	
	string message = "From " + to_string(sourcePID) + "\nto " + to_string(pID) + "\nat " + createdTime + "content= " + m;
	connect(loggerSocket.sock, (sockaddr*)&loggerSocket.address, sizeof(loggerSocket.address));
	char buffer[2048];
	int bufferLeng = message.size();
	strcpy(buffer, message.c_str());
	send(loggerSocket.sock, buffer, bufferLeng, 0);
	cm.unlock();

}
Message messageDeFormatter(char* message)
{
	string mess(message);
	Message ret;
	vector <string>splitted = split(mess);
	ret.sender.pID = stoi(splitted[0]);
	ret.sender.address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//splitted[1]
	ret.sender.address.sin_port = stoi(splitted[2]);
	ret.sender.address.sin_family = AF_INET;
	ret.content = splitted[3];

	time_t now = time(0);
	char* createdTime = ctime(&now);

	ret.recievedTime = createdTime;
	return ret;
}
void addToKnowns(ModiefiedProcess p)
{


	if (p.pID > pID)
	{
		for (int i = 0; i < higher.size(); i++)
		{
			if (higher[i].pID == p.pID && higher[i].address.sin_port == p.address.sin_port)
				return;
			else if (higher[i].pID == p.pID || higher[i].address.sin_port == p.address.sin_port)
			{
				higher[i].pID = p.pID;
				higher[i].address.sin_port = p.address.sin_port;
				coordinator.pID = -1;
				return;
			}
		}
		coordinator.pID = -1;
		higher.push_back(p);
	}
	else if (p.pID < pID)
	{
		for (int i = 0; i < lower.size(); i++)
		{
			if (lower[i].pID == p.pID && lower[i].address.sin_port == p.address.sin_port)
				return;
			if (lower[i].pID == p.pID || lower[i].address.sin_port == p.address.sin_port)
			{
				lower[i].pID = p.pID;
				lower[i].address.sin_port = p.address.sin_port;
				return;
			}
		}
		lower.push_back(p);
	}
}

void sendIntroduce()
{

	ModiefiedSocket introducerSocket;
	introducerSocket.sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (introducerSocket.sock < 0)
	{
		//cout << "ERROR OPENING THE Introducer SOCKET \n";
		return;
	}
	introducerSocket.address.sin_family = AF_INET;
	introducerSocket.address.sin_port = htons(4321);
	introducerSocket.address.sin_addr.S_un.S_addr = inet_addr("239.255.255.250");

	/*
	in_addr localInterface;
	localInterface.S_un.S_addr = inet_addr("239.255.255.250");
	if (setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char*)&localInterface, sizeof(localInterface)) < 0)
	{
		cout << "Setting IP_MULTICAST_IF ERRROR!!!!\n";
		closesocket(sd);
		return 1;
	}*/
	while (1)
	{
		char messageBuffer[2048];
		messageFormater("INTRO", messageBuffer);
		int messageBufferLen = sizeof(messageBuffer);


		if (sendto(introducerSocket.sock, messageBuffer, messageBufferLen, 0, (sockaddr*)&introducerSocket.address, sizeof(introducerSocket.address)) < 0)
		{
			//cout << "ERROR AT SENDing";
			closesocket(introducerSocket.sock);
			return;
		}
		//this_thread::sleep_for(chrono::milliseconds(100));
	}
	closesocket(introducerSocket.sock);
}
void receiveIntroduce()
{
	ModiefiedSocket listenerSocket;
	listenerSocket.sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (listenerSocket.sock < 0)
	{
		//cout << "ERROR OPENING introducing listener SOCKET \n";
		return;
	}

	int reuse = 1;
	if (setsockopt(listenerSocket.sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0)
	{
		//cout << "Setting SO_REUSEADDR error\n";
		closesocket(listenerSocket.sock);
		return;
	}

	listenerSocket.address.sin_addr.s_addr = INADDR_ANY;
	listenerSocket.address.sin_family = AF_INET;
	listenerSocket.address.sin_port = htons(4321);
	if (bind(listenerSocket.sock, (SOCKADDR*)&listenerSocket.address, sizeof(listenerSocket.address)))
	{
		//cout << "ERROR Binding datagram socket";
		closesocket(listenerSocket.sock);
		return;
	}

	ip_mreq group;
	group.imr_multiaddr.S_un.S_addr = inet_addr("239.255.255.250");
	group.imr_interface.S_un.S_addr = htonl(INADDR_ANY);
	if (setsockopt(listenerSocket.sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group, sizeof(group)) < 0)
	{
		//cout << "ERROR AT ADDING MEMBERSHIP" << GetLastError();
		closesocket(listenerSocket.sock);
		return;
	}


	while (1)
	{
		char databuffer[2048];
		int datalen = sizeof(databuffer);

		if (recv(listenerSocket.sock, databuffer, datalen, 0) == SOCKET_ERROR)
		{
			//cout << "ERROR WHILE recieving\n";
			break;
		}
		else
		{
			Message message = messageDeFormatter(databuffer);
			if (message.content == "INTRO")
			{
				addToKnowns(message.sender);
			}
		}
	}
	closesocket(listenerSocket.sock);
}
void init()
{
	pID = GetCurrentProcessId();

	int WSok = WSAStartup(MAKEWORD(2, 2), &WSAData);

	if (WSok != 0)
	{
		cerr << "Can't init WSok at process" << pID << endl;
		exit(1);
	}
	coordinator.pID = -1;
	recieverSocket = createTCPSocket();
	recievingPort = recieverSocket.address.sin_port;
	senderSocket = createTCPSocket();
	loggerSocket = createLoggerSocket();
}

void electionReciever()
{
	listen(recieverSocket.sock, SOMAXCONN);
	char buffer[2048];
	while (1)
	{
		ModiefiedSocket sender;
		int senderSize = sizeof(sender.address);
		sender.sock = accept(recieverSocket.sock, (sockaddr*)&sender.address, &senderSize);
		int byteRecieved = recv(sender.sock, buffer, sizeof(buffer), 0);
		Message message = messageDeFormatter(buffer);
		//cout <<pID <<  " Recieved message = " << message.content << " FROM " << message.sender.pID<<'\n';

		sendToLogger(message.content, message.sender.pID);
		if (message.content == "ELECTION")
		{
			//send ok (actually not needed)

			//set coordinator as null
			coordinator.pID = -1;

		}
		else if (message.content == "NEWCOORDINATOR")
		{
			//set sender as coordinator
			coordinator = message.sender;
			//cout << "PROCESS #" << pID << " KNOWS THAT PROCESS #" << coordinator.pID << " IS THE COORDINATOR\n";
		}
		else if (message.content == "ALIVE?")
		{
			//send yes
			//not needed 
		}

	}
}
bool checkIfCoordinatorIsAlive()
{
	if (coordinator.pID == -1)
		return false;
	if (coordinator.pID == pID)
		return true;
	senderSocket.address = coordinator.address;
	int connectionResult =
		connect(senderSocket.sock, (sockaddr*)&senderSocket.address, sizeof(senderSocket.address));
	//cout << "CHECKING COORDINATOR # " << coordinator.pID << " At port # " << coordinator.address.sin_port << " from = " << pID<<endl;
	char buffer[2048];
	int bufferLeng = sizeof(buffer);
	messageFormater("ALIVE?", buffer);
	int sendResult = send(senderSocket.sock, buffer, bufferLeng, 0);

	if (sendResult != SOCKET_ERROR)
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool sendElectionTo(ModiefiedProcess p)
{
	senderSocket.address = p.address;
	int connectionResult =
		connect(senderSocket.sock, (sockaddr*)&senderSocket.address, sizeof(senderSocket.address));

	char buffer[2048];
	int bufferLeng = sizeof(buffer);
	messageFormater("ELECTION", buffer);
	int sendResult = send(senderSocket.sock, buffer, bufferLeng, 0);

	if (sendResult != SOCKET_ERROR)
	{
		return true;
	}
	else
	{
		//cout << pID << "couldn't send to " << p.pID << "\n";
		return false;
	}
}
void sendIamTheNewCoordinator()
{
	coordinator.pID = pID;
	coordinator.address = recieverSocket.address;

	for (int i = 0; i < lower.size(); i++)
	{
		coordinator.pID = pID;
		coordinator.address = recieverSocket.address;
		char buffer[2048];
		int bufferLeng = sizeof(buffer);
		messageFormater("NEWCOORDINATOR", buffer);
		senderSocket.address = lower[i].address;
		int connectionResult =
			connect(senderSocket.sock, (sockaddr*)&senderSocket.address, sizeof(senderSocket.address));
		int sendRes = send(senderSocket.sock, buffer, bufferLeng, 0);
		/*if (sendRes )
		{
			cout << "iam " << pID << " sent iam leader to " << lower[i].pID << '\n';
		}*/
	}
}
void electionSender()
{

	while (1)
	{
		//check if the cooordinator still alive
		//else
		//search for new coordinator
		if (checkIfCoordinatorIsAlive())
		{
			this_thread::sleep_for(chrono::seconds(1));
		}
		else
		{
			int highest = -1;
			for (int i = 0; i < higher.size(); i++)
			{
				if (higher[i].pID > highest&& sendElectionTo(higher[i]))
				{
					//cout << "node = " << pID << " sent election to " << higher[i].pID << '\n';
					highest = higher[i].pID;
				}
			}
			if ( highest == -1)
			{
				sendIamTheNewCoordinator();
			}
			this_thread::sleep_for(chrono::seconds(1));
		}

	}
}
int main(int argc, char* argv[])
{

	init();
	thread t2(receiveIntroduce);
	thread t1(sendIntroduce);
	thread t3(electionReciever);
	thread t4(electionSender);
	while (true)
	{
		/*for (int i = 0; i < higher.size(); i++)
		{
			cout << "proccess #" << _getpid() << " knows that process #" << higher[i].pID<<" is higher\n";
		}
		for (int i = 0; i < lower.size(); i++)
		{
			cout << "proccess #" << _getpid() << " knows that process #" << lower[i].pID << " is lower\n";
		}
		*/
		cout << pID << " COORDINATOR = " << coordinator.pID << '\n';
		this_thread::sleep_for(std::chrono::seconds(2));
	}

}