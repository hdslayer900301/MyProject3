#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Math/UnrealMathUtility.h"
#include "TbTNav.generated.h"

UCLASS()
class MYPROJECT3_API UTbTNav : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Turn By Turn Navigation")
		static TArray<FVector> CalculatePath(FString mapFile, FString origin, FString destination);

};