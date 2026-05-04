#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "VDAClientComponent.generated.h"

class FVDA5050Client;

USTRUCT(BlueprintType)
struct VDACLIENTPLUGIN_API FVDA5050NodeInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "VDA5050")
    FString NodeId;

    UPROPERTY(BlueprintReadOnly, Category = "VDA5050")
    int32 SequenceId = 0;

    UPROPERTY(BlueprintReadOnly, Category = "VDA5050")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "VDA5050")
    float Theta = 0.0f;
};

USTRUCT(BlueprintType)
struct VDACLIENTPLUGIN_API FVDA5050OrderInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "VDA5050")
    FString OrderId;

    UPROPERTY(BlueprintReadOnly, Category = "VDA5050")
    int32 OrderUpdateId = 0;

    UPROPERTY(BlueprintReadOnly, Category = "VDA5050")
    TArray<FVDA5050NodeInfo> Nodes;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNodeDispatch, const FVDA5050NodeInfo &, Node);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOrderReceived, const FVDA5050OrderInfo &, Order);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectComplete, bool, bSuccess);

UCLASS( ClassGroup=(VDA5050), meta =(BlueprintSpawnableComponent) )
class VDACLIENTPLUGIN_API UVDAClientComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVDAClientComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VDA5050|Connection")
    FString BrokerAddress = TEXT("tcp://localhost:1883");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VDA5050|Connection")
    FString InterfaceName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VDA5050|Connection")
    FString Version = TEXT("2.0.0");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VDA5050|Connection")
    FString Manufacturer = TEXT("Manufacturer");

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "VDA5050|Connection")
    FString SerialNumber = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VDA5050|Connection")
    bool bAutoConnect = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VDA5050|State")
    bool bPublishState = true;

    UPROPERTY(BlueprintAssignable, Category = "VDA5050|Events")
    FOnNodeDispatch OnNodeDispatch;

    UPROPERTY(BlueprintAssignable, Category = "VDA5050|Events")
    FOnOrderReceived OnOrderReceived;

    UPROPERTY(BlueprintAssignable, Category = "VDA5050|Events")
    FOnConnectComplete OnConnectComplete;

    UFUNCTION(BlueprintCallable, Category = "VDA5050",
    meta = (DisplayName = "Connect VDAClient", Keywords = "Establish client connection"))
    void Connect(
        const FString & InBrokerAddress = "tcp://localhost:1883",
        const FString & InInterfaceName = "",
        const FString & InVersion = "1.0.0",
        const FString & InManufacturer = "Manufacturer",
        const FString & InSerialNumber = "");

    UFUNCTION(BlueprintCallable, Category = "VDA5050",
    meta = (DisplayName = "Disconnect VDAClient", Keywords = "Disconnect client"))
    void Disconnect();

    UFUNCTION(BlueprintCallable, Category = "VDA5050",
    meta = (DisplayName = "Acknowledge Node", Keywords = "Acknowledge VDA Node Completion"))
    void AcknowledgeNode(int32 SequenceId);

protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    TSharedPtr<FVDA5050Client> Client;
};
