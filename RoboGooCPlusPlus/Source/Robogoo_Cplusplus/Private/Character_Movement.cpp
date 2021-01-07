#include "Character_Movement.h"
#include "Bullet.h"

// Sets default values
ACharacter_Movement::ACharacter_Movement()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 800.0f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->GravityScale = 2.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	FollowCamera->bUsePawnControlRotation = false;


	GooSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GooObject"));
	GooSphere->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	GooSphere->SetStaticMesh(SphereMeshAsset.Object);

	ConstructorHelpers::FObjectFinder<UMaterial> plane_material(TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial'"));		// Standard material
	GooSphere->GetStaticMesh()->SetMaterial(0, plane_material.Object);


	GooShield = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GooShield"));
	GooShield->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset1(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	GooShield->SetStaticMesh(SphereMeshAsset1.Object);

	ConstructorHelpers::FObjectFinder<UMaterial> plane_material1(TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial'"));		// Standard material
	GooShield->GetStaticMesh()->SetMaterial(0, plane_material1.Object);


	shootpoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("shootpoint"));
	shootpoint->SetupAttachment(RootComponent);

	damagedist = 200;
	PlayerHealth = 100;
}

// Called when the game starts or when spawned
void ACharacter_Movement::BeginPlay()
{
	Super::BeginPlay();

	GooSphere->SetWorldScale3D(FVector(0.75f, 0.75f, 0.75f));
	GooSphere->ToggleVisibility(false);

	GooShield->SetWorldScale3D(FVector(0.2f, 1.f, 1.f));
	GooShield->SetRelativeLocation(FVector(40.f, 0.f, 30.f));
	GooShield->ToggleVisibility(false);

	landed = true;
	glidenum = 0;
}

// Called every frame
void ACharacter_Movement::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	height = GetActorLocation().X;

	if (aim)
	{
		CameraBoom->TargetArmLength = 100.0f;
		CameraBoom->SetRelativeLocation(FMath::Lerp(CameraBoom->GetRelativeLocation(), FVector(0.f, 30.f, 70.f), 0.5f));

		bUseControllerRotationRoll = true;
		bUseControllerRotationYaw = true;
	}
	else
	{
		CameraBoom->TargetArmLength = 300.0f;
		CameraBoom->SetRelativeLocation(FMath::Lerp(CameraBoom->GetRelativeLocation(), FVector(0.f, 0.f, 0.f), 0.5f));

		bUseControllerRotationRoll = false;
		bUseControllerRotationYaw = false;
	}
}

// Called to bind functionality to input
void ACharacter_Movement::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);		
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter_Movement::Jumpglide);

	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter_Movement::Stopglide);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACharacter_Movement::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACharacter_Movement::MoveRight);

	PlayerInputComponent->BindAction("GooTrigger", IE_Pressed, this, &ACharacter_Movement::DisableActor);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACharacter_Movement::OnFire);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ACharacter_Movement::Aiming);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ACharacter_Movement::AimReset);

	PlayerInputComponent->BindAction("Block", IE_Pressed, this, &ACharacter_Movement::Blocking);
	PlayerInputComponent->BindAction("Block", IE_Released, this, &ACharacter_Movement::BlockReset);

}

void ACharacter_Movement::MoveForward(float Axis)
{
	FRotator Rotation = Controller->GetControlRotation();
	FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

	FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, Axis);
}

void ACharacter_Movement::MoveRight(float Axis)
{
	FRotator Rotation = Controller->GetControlRotation();
	FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

	FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Direction, Axis);
}

void ACharacter_Movement::DisableActor()
{
	if (flip == false) flip = true;
	else if (flip == true) flip = false;

	//flip = flip ? true : false;

	GooSphere->ToggleVisibility(flip);

	if (flip)
	{
		GetCharacterMovement()->JumpZVelocity = 700.0f;
		GetCharacterMovement()->GravityScale = 0.8f;

		landed = false;
	}
	else
	{
		GetCharacterMovement()->JumpZVelocity = 800.0f;
		GetCharacterMovement()->GravityScale = 2.f;
	}

	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, FString::Printf(TEXT("Bool: %s"), flip ? TEXT("true") : TEXT("false")));
}

void ACharacter_Movement::OnFire()
{
	if (ProjectileClass != NULL && block == false)
	{
		UWorld* const World = GetWorld();

		const FRotator SpawnRotation = ((shootpoint != nullptr && aim == true) ? GetControlRotation() : GetActorRotation());

		const FVector SpawnLocation = ((shootpoint != nullptr && aim == true) ? shootpoint->GetComponentLocation() : GetActorLocation() + (GetActorForwardVector() * 60));

		if (World != NULL)
		{
			ABullet* Bullet = World->SpawnActor<ABullet>(ProjectileClass, SpawnLocation, SpawnRotation);

			FVector NewVelocity = GetActorForwardVector() * 5000.f;

			Bullet->Velocity = FVector(NewVelocity);

			if (aim == true)
			{
				Bullet->aimlong = true;
			}
		}
	}
}

void ACharacter_Movement::Aiming()
{
	aim = true;
}

void ACharacter_Movement::AimReset()
{
	aim = false;
}

void ACharacter_Movement::Blocking()
{
	//aim = true;
	GooShield->ToggleVisibility(true);
}

void ACharacter_Movement::BlockReset()
{
	//aim = false;
	GooShield->ToggleVisibility(false);
}

void ACharacter_Movement::Landed(const FHitResult& Hit)
{
	Super::OnLanded(Hit);

	landed = true;
	glidenum = 0;

	if (flip)
	{
		GetCharacterMovement()->JumpZVelocity = 700.0f;
		GetCharacterMovement()->GravityScale = 0.8f;
	}
	else
	{
		GetCharacterMovement()->JumpZVelocity = 800.0f;
		GetCharacterMovement()->GravityScale = 2.f;
	}

	if (height - (damagedist + heightoffset))
	{
		float damage = ((height - (damagedist + heightoffset)));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::SanitizeFloat(damage));

		if (((height - (damagedist + heightoffset)) > 101.f) && ((height - (damagedist + heightoffset))) < 200.f) PlayerHealth -= 1.f;

		if (((height - (damagedist + heightoffset)) > 201.f) && ((height - (damagedist + heightoffset))) < 500.f)
		{
			float tempdamage = ((height - (damagedist + heightoffset)) * 0.1f);
			int damageint = std::round(tempdamage);

			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::SanitizeFloat(damageint));

			PlayerHealth -= damageint;
		}

		if ((height - (damagedist + heightoffset)) > 501.f) PlayerHealth -= 100.f;

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::SanitizeFloat(PlayerHealth));
	}

	if (PlayerHealth <= 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("DEATH"));
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("landed"));
}

void ACharacter_Movement::Jumpglide()
{
	heightoffset = GetActorLocation().X;

	if (flip)
	{
		landed = false;
		glidenum++;

		if (!landed && glidenum == 2)
		{
			glidenum = 3;

			GetCharacterMovement()->Velocity = FVector(GetCharacterMovement()->Velocity.X, GetCharacterMovement()->Velocity.Y, 0);

			GetCharacterMovement()->GravityScale = 0.2f;

		}
	}
}

void ACharacter_Movement::Stopglide()
{
	if (glidenum >= 3)
	{
		GetCharacterMovement()->GravityScale = 0.8f;
	}
}