// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerChar.h"

// Sets default values
APlayerChar::APlayerChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Cam Setup
	PlayerCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Cam"));

	//attach camera to mesh
	PlayerCamComp->SetupAttachment(GetMesh(), "head");

	//using controller rotation for character
	PlayerCamComp->bUsePawnControlRotation = true;

	//Initialize resources array and names
	ResourcesArray.SetNum(3);
	ResourcesNameArray.Add(TEXT("Wood"));
	ResourcesNameArray.Add(TEXT("Stone"));
	ResourcesNameArray.Add(TEXT("Berry"));


}

// Called when the game starts or when spawned
void APlayerChar::BeginPlay()
{
	Super::BeginPlay();

	//Add timer for stats decrease on player every 2 seconds
	FTimerHandle StatsTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(StatsTimerHandle, this, &APlayerChar::DecreaseStats, 2.0f, true);
    
}

// Called every frame
void APlayerChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//call base class to ensure parent setup is completed
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//bind axis and action mappings
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerChar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerChar::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerChar::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APlayerChar::AddControllerYawInput);
	PlayerInputComponent->BindAction("JumpEvent", IE_Pressed, this, &APlayerChar::StartJump);
	PlayerInputComponent->BindAction("JumpEvent", IE_Released, this, &APlayerChar::StopJump);

	//Bind interact action to FindObject function
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerChar::FindObject);


}

//handles forward.backward movement
void APlayerChar::MoveForward(float axisValue)
{
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	AddMovementInput(Direction, axisValue);
}

//handles right/left movement
void APlayerChar::MoveRight(float axisValue)
{
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(Direction, axisValue);	
}

//called when jump button is pressed
void APlayerChar::StartJump()
{
	bPressedJump = true;
}

void APlayerChar::StopJump()
{
	bPressedJump = false;
}

//Function to find resource object in front of player and interact with it
void APlayerChar::FindObject()
{
	FHitResult HitResult;
	FVector StartLocation = PlayerCamComp->GetComponentLocation();
	FVector Direction = PlayerCamComp->GetForwardVector() * 800.0f;
	FVector EndLocation = StartLocation + Direction;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnFaceIndex = true;

	//Line trace to find resource object
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
	{
		AResource_M* HitResource = Cast<AResource_M>(HitResult.GetActor());

		//If resource object found, give player resources and decrease resource object's total resources
		if (HitResource)
		{
			FString hitName = HitResource->resourceName;
			int resourceValue = HitResource->resourceAmount;

			HitResource->totalResource = HitResource->totalResource - resourceValue;
			
			//Only give resources if resource object has enough total resources
			if (HitResource->totalResource > resourceValue)
			{
				GiveResource(resourceValue, hitName);

				check(GEngine != nullptr);
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Resource Collected"));
			}
			//If resource object does not have enough resources, destroy it
			else
			{
				HitResource->Destroy();
				check(GEngine != nullptr);
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Resource Depleted"));
			}
		}

	}
}

//Functions to modify player stats
void APlayerChar::SetHealth(float amount)
{
	//Ensure health does not exceed 100
	if (Health + amount < 100)
	{
		Health = Health + amount;
	}
}

void APlayerChar::SetHunger(float amount)
{
	//Ensure hunger does not exceed 100
	if (Hunger + amount < 100)
	{
		Hunger = Hunger + amount;
	}
}

void APlayerChar::SetStamina(float amount)
{
	//Ensure stamina does not exceed 100
	if (Stamina + amount < 100)
	{
		Stamina = Stamina + amount;
	}
}

void APlayerChar::DecreaseStats()
{
	//Decrease hunger by 1 every 2 seconds
	if (Hunger > 0)
	{
		SetHunger(-1.0f);
	}
	
	SetStamina(10.0f);
	//If hunger is 0, decrease health by 3 every 2 seconds
	if (Hunger <= 0)
	{
		SetHealth(-3.0f);
	}
}

//Function to give resources to player
void APlayerChar::GiveResource(float amount, FString resourceType)
{
	//Check resource type and add amount to corresponding resource in resources array
	if (resourceType == "Wood")
	{
		ResourcesArray[0] = ResourcesArray[0] + amount;
	}

	if (resourceType == "Stone")
	{
		ResourcesArray[1] = ResourcesArray[1] + amount;
	}

	if (resourceType == "Berry")
	{
		ResourcesArray[2] = ResourcesArray[2] + amount;
	}
}

