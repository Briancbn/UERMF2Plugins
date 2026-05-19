#include "VDAClientComponent.h"
#include "VDAWrapper.h"

#include "Async/Async.h"

static constexpr double CM_TO_M = 100.0;

UVDAClientComponent::UVDAClientComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UVDAClientComponent::BeginPlay()
{
    Super::BeginPlay();

    Client = MakeShared<FVDA5050Client>();

    Client->OnOrderReceived = [this](const FVDA5050Order & Order) {
        FVDA5050OrderInfo Info;
        Info.OrderId = UTF8_TO_TCHAR(Order.OrderId.c_str());
        Info.OrderUpdateId = Order.OrderUpdateId;
        for (const auto & Node : Order.Nodes)
        {
            FVDA5050NodeInfo NodeInfo;
            NodeInfo.NodeId = UTF8_TO_TCHAR(Node.NodeId.c_str());
            NodeInfo.SequenceId = Node.SequenceId;
            // Convert from UE5's coordinate system to VDA standard.
            NodeInfo.Position = FVector(Node.X * CM_TO_M, Node.Y * CM_TO_M, GetOwner()->GetActorLocation().Z);
            // The current UE5 controller does not take heading angle into account. As of now the Theta value is not in use. 
            // TODO(DillonChew98): Make controller utilise theta value if specified.
            NodeInfo.Theta = Node.Theta.value_or(0);
            Info.Nodes.Add(NodeInfo);
        }
        OnOrderReceived.Broadcast(Info);
    };

    Client->OnNodeDispatch = [this](const FVDA5050Node & Node) {
        FVDA5050NodeInfo NodeInfo;
        NodeInfo.NodeId = UTF8_TO_TCHAR(Node.NodeId.c_str());
        NodeInfo.SequenceId = Node.SequenceId;
        NodeInfo.Position = FVector(Node.X * CM_TO_M, Node.Y * CM_TO_M, GetOwner()->GetActorLocation().Z);
        NodeInfo.Theta = Node.Theta.value_or(0);  
        OnNodeDispatch.Broadcast(NodeInfo);
    };

    // Sends back current position of robot to the VDA Client.
    Client->OnPositionRequest = [this](double & X, double & Y, double & Theta) {
        FVector Pos = GetOwner()->GetActorLocation();
        FRotator Rot = GetOwner()->GetActorRotation();
        X = Pos.X / CM_TO_M;
        Y = Pos.Y / CM_TO_M;
        Theta = FMath::DegreesToRadians(Rot.Yaw);
    };

    if (bAutoConnect)
    {
        Connect(BrokerAddress, InterfaceName, Version, Manufacturer, SerialNumber);
    }
}

void UVDAClientComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Disconnect();
    Client.Reset();
    Super::EndPlay(EndPlayReason);
}

void UVDAClientComponent::Connect(
    const FString & InBrokerAddress,
    const FString & InInterfaceName,
    const FString & InVersion,
    const FString & InManufacturer,
    const FString & InSerialNumber
)
{
    if (!Client) return;

    FString broker_address = InBrokerAddress;
    FString interface_name = InInterfaceName;
    FString version = InVersion;
    FString manufacturer = InManufacturer;
    FString serial_number = InSerialNumber;

    // Might be a better idea to specify specific threads to run this on. As of now no apparent implicationss using AnyThread
    AsyncTask(ENamedThreads::AnyThread, [this, broker_address, interface_name, version, manufacturer, serial_number]()
    {
        // This code will run asynchronously, without freezing the game thread.
        bool bSuccess = Client->Connect(
            TCHAR_TO_UTF8(*broker_address),
            TCHAR_TO_UTF8(*interface_name),
            TCHAR_TO_UTF8(*version),
            TCHAR_TO_UTF8(*manufacturer),
            TCHAR_TO_UTF8(*serial_number)
        );
        if (bSuccess)
        {
            Client->SetPublishState(bPublishState);
        }
        // TODO(DillonChew98): Add case to handle connection failure.
        // while (!bSuccess) {}
        AsyncTask(ENamedThreads::GameThread, [this, bSuccess]()
        {
            OnConnectComplete.Broadcast(bSuccess);
        });
    });
}

void UVDAClientComponent::Disconnect()
{
    if (Client)
    {
        Client->Disconnect();
    }
}

void UVDAClientComponent::AcknowledgeNode(int32 SequenceId)
{
    if (Client)
    {
        Client->ClientNodeAck(static_cast<uint32_t>(SequenceId));
    }
}

void UVDAClientComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (Client)
    {
        Client->SpinOnce();
    }
}