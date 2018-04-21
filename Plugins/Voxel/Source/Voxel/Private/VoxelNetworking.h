// Copyright 2018 Phyronnaz

#pragma once


#include "CoreMinimal.h"
#include "VoxelDiff.h"
#include "VoxelSave.h"
#include "IPv4Endpoint.h"

#define PACKET_SIZE_IN_DIFF 250

DECLARE_DELEGATE(FOnVoxelTcpServerConnection)

class FSocket;
class FTcpListener;
class AVoxelWorld;

/**
 * The client can only receive diffs and saves
 */
class FVoxelTcpClient
{
public:
	FVoxelTcpClient();
	~FVoxelTcpClient();

	/**
	 * Connect to the server
	 * @param	Ip		The ip of the server
	 * @param	Port	The port of the server
	 */
	void ConnectTcpClient(const FString& Ip, int32 Port);

	/**
	 * Receive the diffs from the server. Must not be called when IsNextUpdateRemoteLoad is true
	 */
	void ReceiveDiffQueues(TArray<FVoxelValueDiff>& OutValueDiffQueue, TArray<FVoxelMaterialDiff>& OutMaterialDiffQueue);

	/**
	 * Receive the save from the server. Must not be called when IsNextUpdateRemoteLoad is false
	 */
	void ReceiveSave(FVoxelWorldSave& OutSave);

	/**
	 * Is the connection valid?
	 */
	bool IsValid() const;

	/**
	 * Is the server sending the save?
	 */
	bool IsNextUpdateRemoteLoad();

	/**
	 * Check if headers has been received. Should be called before IsNextUpdateRemoteLoad
	 */
	void UpdateExpectedSize();

private:
	FSocket * Socket;

	bool bExpectedSizeUpToDate;
	uint32 ExpectedSize;
	bool bNextUpdateIsRemoteLoad;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * The server can send the diffs and the saves
 */
class FVoxelTcpServer
{
public:
	FOnVoxelTcpServerConnection OnConnectionDelegate;

	FVoxelTcpServer();
	~FVoxelTcpServer();

	/** Delegate */
	FOnVoxelTcpServerConnection& OnConnection();

	/**
	 * Start the server
	 * @param	Ip		The ip of the server. Most of the time its ip on its local network (not 127.0.0.1)
	 * @param	Port	The port of the server
	 */
	void StartTcpServer(const FString& Ip, int32 Port);

	/**
	 * Has the server started?
	 */
	bool IsValid();

	/**
	 * Send the value diffs
	 */
	void SendValueDiffQueue(const TArray<FVoxelValueDiff>& ValueDiffQueue);
	/**
	 * Send the materials diffs
	 */
	void SendMaterialDiffQueue(const TArray<FVoxelMaterialDiff>& MaterialDiffQueue);
	/**
	 * Send a world save
	 * @param	bOnlyToNewConnections	Only send save to the one that haven't synced yet
	 */
	void SendSave(const FVoxelWorldSave& Save, bool bOnlyToNewConnections);

private:
	FTcpListener* TcpListener;
	TArray<FSocket*> Sockets;
	TArray<FSocket*> SocketsToSendSave;
	FCriticalSection SocketsLock;

	template<typename T>
	void SendDiffs(const TArray<T>& DiffList, bool bIsValues);

	/**
	 * Callback from the server
	 */
	bool Accept(FSocket* NewSocket, const FIPv4Endpoint& Endpoint);
};
