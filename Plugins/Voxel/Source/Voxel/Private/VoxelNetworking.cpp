// Copyright 2018 Phyronnaz

#include "VoxelNetworking.h"
#include "VoxelPrivate.h"
#include "Sockets.h"
#include "TcpListener.h"

#include "ArchiveSaveCompressedProxy.h"
#include "ArchiveLoadCompressedProxy.h"
#include "Engine.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelTcpClient::ReceiveData"), STAT_FVoxelTcpClient_ReceiveData, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelTcpServer::SendData"), STAT_FVoxelTcpServer_SendData, STATGROUP_Voxel);

FVoxelTcpClient::FVoxelTcpClient()
	: Socket(nullptr)
	, ExpectedSize(0)
	, bExpectedSizeUpToDate(false)
{

}

FVoxelTcpClient::~FVoxelTcpClient()
{
	if (Socket)
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	}
}

void FVoxelTcpClient::ConnectTcpClient(const FString& Ip, const int32 Port)
{
	//Create Remote Address.
	TSharedPtr<FInternetAddr> RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	bool bIsValid;
	RemoteAddr->SetIp(*Ip, bIsValid);
	RemoteAddr->SetPort(Port);

	if (!bIsValid)
	{
		UE_LOG(LogVoxel, Error, TEXT("IP address was not valid"));
		return;
	}

	FIPv4Endpoint Endpoint(RemoteAddr);

	if (Socket)
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	}

	Socket = FTcpSocketBuilder(TEXT("RemoteConnection"));

	int BufferSize = 1000000;
	int NewSize;
	Socket->SetReceiveBufferSize(BufferSize, NewSize);
	check(BufferSize == NewSize);

	if (Socket)
	{
		if (!Socket->Connect(*Endpoint.ToInternetAddr()))
		{
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
			Socket = nullptr;
			return;
		}
	}
}

void FVoxelTcpClient::ReceiveDiffQueues(TArray<FVoxelValueDiff>& OutValueDiffs, TArray<FVoxelMaterialDiff>& OutMaterialDiffs)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelTcpClient_ReceiveData);

	check(!bNextUpdateIsRemoteLoad);

	if (Socket)
	{
		if (bExpectedSizeUpToDate)
		{
			uint32 PendingDataSize = 0;
			if (Socket->HasPendingData(PendingDataSize))
			{
				if (PendingDataSize >= ExpectedSize)
				{
					TArray<uint8> ReceivedData;
					ReceivedData.SetNumUninitialized(ExpectedSize);

					int32 BytesRead = 0;
					Socket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead);
					check(BytesRead == ExpectedSize);

					FArchiveLoadCompressedProxy Decompressor = FArchiveLoadCompressedProxy(ReceivedData, ECompressionFlags::COMPRESS_ZLIB);
					check(!Decompressor.GetError());

					//Decompress
					FBufferArchive DecompressedDataArray;
					Decompressor << DecompressedDataArray;

					FMemoryReader DecompressedData = FMemoryReader(DecompressedDataArray);

					bool bValues;
					uint32 ItemCount;
					DecompressedData << bValues;
					DecompressedData << ItemCount;

					check(ItemCount <= PACKET_SIZE_IN_DIFF);

					if (bValues)
					{
						for (uint32 i = 0; i < ItemCount; i++)
						{
							FVoxelValueDiff Diff;
							DecompressedData << Diff;
							OutValueDiffs.Add(Diff);
						}
					}
					else
					{
						for (uint32 i = 0; i < ItemCount; i++)
						{
							FVoxelMaterialDiff Diff;
							DecompressedData << Diff;
							OutMaterialDiffs.Add(Diff);
						}
					}

					bExpectedSizeUpToDate = false;
					UpdateExpectedSize();
				}
			}
		}
	}
	else
	{
		UE_LOG(LogVoxel, Error, TEXT("Client not connected"));
	}
}

void FVoxelTcpClient::ReceiveSave(FVoxelWorldSave& OutSave)
{
	check(bNextUpdateIsRemoteLoad);

	if (Socket)
	{
		if (bExpectedSizeUpToDate)
		{
			uint32 PendingDataSize = 0;
			if (Socket->HasPendingData(PendingDataSize))
			{
				if (PendingDataSize >= ExpectedSize)
				{
					FBufferArchive ReceivedData(true);
					ReceivedData.SetNumUninitialized(ExpectedSize);

					int32 BytesRead = 0;
					Socket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead);
					bool bSuccess = BytesRead == ExpectedSize;
					UE_LOG(LogVoxel, Log, TEXT("Remote load: Bytes to receive: %d. Bytes received: %d. Success: %d"), ExpectedSize, BytesRead, bSuccess);
					if (bSuccess)
					{

						FMemoryReader Reader(ReceivedData);
						Reader << OutSave.LOD;
						Reader << OutSave.Data;

						bNextUpdateIsRemoteLoad = false;
						bExpectedSizeUpToDate = false;
						UpdateExpectedSize();
					}
					else
					{
						UE_LOG(LogVoxel, Error, TEXT("Remote load: Fail"));
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LogVoxel, Error, TEXT("Client not connected"));
	}
}

bool FVoxelTcpClient::IsValid() const
{
	return Socket != nullptr;
}

bool FVoxelTcpClient::IsNextUpdateRemoteLoad()
{
	return bNextUpdateIsRemoteLoad;
}

void FVoxelTcpClient::UpdateExpectedSize()
{
	if (!bExpectedSizeUpToDate)
	{
		uint32 PendingDataSize = 0;
		if (Socket->HasPendingData(PendingDataSize))
		{
			if (PendingDataSize >= 5)
			{
				uint8 ReceivedData[5];

				int BytesRead;
				Socket->Recv(ReceivedData, 5, BytesRead);
				check(BytesRead == 5);

				ExpectedSize = ReceivedData[0] + 256 * (ReceivedData[1] + 256 * (ReceivedData[2] + 256 * ReceivedData[3]));
				bNextUpdateIsRemoteLoad = ReceivedData[4];

				bExpectedSizeUpToDate = true;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

FVoxelTcpServer::FVoxelTcpServer()
	: TcpListener(nullptr)
{

}

FVoxelTcpServer::~FVoxelTcpServer()
{
	for (auto Socket : Sockets)
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	}
	delete TcpListener;
}

FOnVoxelTcpServerConnection& FVoxelTcpServer::OnConnection()
{
	return OnConnectionDelegate;
}

void FVoxelTcpServer::StartTcpServer(const FString& Ip, const int32 Port)
{
	if (TcpListener)
	{
		delete TcpListener;
	}

	FIPv4Address Addr;
	FIPv4Address::Parse(Ip, Addr);

	FIPv4Endpoint Endpoint(Addr, Port);

	TcpListener = new FTcpListener(Endpoint);

	TcpListener->OnConnectionAccepted().BindRaw(this, &FVoxelTcpServer::Accept);
}

bool FVoxelTcpServer::Accept(FSocket* NewSocket, const FIPv4Endpoint& Endpoint)
{
	FScopeLock Lock(&SocketsLock);

	OnConnectionDelegate.ExecuteIfBound();

	Sockets.Add(NewSocket);
	SocketsToSendSave.Add(NewSocket);

	int BufferSize = 1000000;
	int NewSize;
	NewSocket->SetSendBufferSize(BufferSize, NewSize);
	check(BufferSize == NewSize);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Connected!"));

	return true;
}

bool FVoxelTcpServer::IsValid()
{
	FScopeLock Lock(&SocketsLock);
	return Sockets.Num() > 0;
}

void FVoxelTcpServer::SendValueDiffQueue(const TArray<FVoxelValueDiff>& Diffs)
{
	SendDiffs(Diffs, true);
}

void FVoxelTcpServer::SendMaterialDiffQueue(const TArray<FVoxelMaterialDiff>& Diffs)
{
	SendDiffs(Diffs, false);
}

void FVoxelTcpServer::SendSave(const FVoxelWorldSave& Save, bool bOnlyToNewConnections)
{
	// Needed because of <<
	FVoxelWorldSave& NotConstSave = const_cast<FVoxelWorldSave&>(Save);
	
	FBufferArchive Writer;
	Writer << NotConstSave.LOD;
	Writer << NotConstSave.Data;

	{
		FScopeLock Lock(&SocketsLock);
		for (auto Socket : (bOnlyToNewConnections ? SocketsToSendSave : Sockets))
		{
			// Send
			int32 BytesSent = 0;

			uint8 Data[5];

			uint32 Tmp = Writer.Num();
			Data[0] = Tmp % 256;
			Tmp /= 256;
			Data[1] = Tmp % 256;
			Tmp /= 256;
			Data[2] = Tmp % 256;
			Tmp /= 256;
			Data[3] = Tmp % 256;
			Data[4] = true;

			Socket->Send(Data, 5, BytesSent);
			if (BytesSent != 5)
			{
				UE_LOG(LogVoxel, Error, TEXT("Header for remote load failed to send"));
			}
			else
			{
				bool bSuccess = Socket->Send(Writer.GetData(), Writer.Num(), BytesSent);
				UE_LOG(LogVoxel, Log, TEXT("Remote load: Bytes to send: %d. Bytes sent: %d. Success: %d"), Writer.Num(), BytesSent, bSuccess);
			}
		}
		SocketsToSendSave.Reset();
	}
}

template<typename T>
void FVoxelTcpServer::SendDiffs(const TArray<T>& DiffList, bool bIsValues)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelTcpServer_SendData);

	int i = 0;
	while (i < DiffList.Num())
	{
		uint32 SizeToSend = FMath::Min<int>(PACKET_SIZE_IN_DIFF, DiffList.Num() - i);

		FBufferArchive Writer;
		Writer << bIsValues;
		Writer << SizeToSend;

		for (uint32 k = 0; k < SizeToSend; k++)
		{
			Writer << const_cast<T&>(DiffList[i]);
			i++;
		}

		TArray<uint8> DataToSend;
		FArchiveSaveCompressedProxy Compressor = FArchiveSaveCompressedProxy(DataToSend, ECompressionFlags::COMPRESS_ZLIB);
		// Send entire binary array/archive to compressor
		Compressor << Writer;
		// Send archive serialized data to binary array
		Compressor.Flush();

		{
			FScopeLock Lock(&SocketsLock);
			for (auto Socket : Sockets)
			{
				// Send
				int32 BytesSent = 0;

				uint8 Data[5];

				uint32 Tmp = DataToSend.Num();
				Data[0] = Tmp % 256;
				Tmp /= 256;
				Data[1] = Tmp % 256;
				Tmp /= 256;
				Data[2] = Tmp % 256;
				Tmp /= 256;
				Data[3] = Tmp % 256;
				Data[4] = false;

				Socket->Send(Data, 5, BytesSent);
				if (BytesSent != 5)
				{
					UE_LOG(LogVoxel, Error, TEXT("Header failed to send"));
				}
				else
				{
					bool bSuccess = Socket->Send(DataToSend.GetData(), DataToSend.Num(), BytesSent);
					UE_LOG(LogVoxel, Log, TEXT("Bytes to send: %d. Bytes sent: %d. Success: %d"), DataToSend.Num(), BytesSent, bSuccess);
				}
			}
		}
	}
}
