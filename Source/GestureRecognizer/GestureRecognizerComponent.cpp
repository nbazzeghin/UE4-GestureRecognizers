// 2014 MartianCraft, LLC

#include "GestureRecognizer.h"
//#include "RepublicSniper.h"
#include "GestureRecognizerComponent.h"


// Sets default values for this component's properties
UGestureRecognizerComponent::UGestureRecognizerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UGestureRecognizerComponent::InitializeComponent()
{
	Super::InitializeComponent();
	ResetGesture();
	
	AActor *Owner = GetOwner();
	if (Owner->IsValidLowLevel())
	{
		AddTickPrerequisiteActor(Owner);
	}
	
	APlayerController *Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (Controller->IsValidLowLevel())
	{
		PlayerInput = Controller->PlayerInput;
	}
}

void UGestureRecognizerComponent::ProcessTouches(const FVector (&Touches)[EKeys::NUM_TOUCH_KEYS], float DeltaTime)
{
	CalculateTouchCount(Touches);
	
	UpdateStoredTouchData(Touches);
	
	// If no touches, return immediately to avoid unnecessary processing
	if (CurrentTouchCount == 0)
	{
		if (PreviousTouchCount > 0)
		{
			// Final processing, then reset touch data. Subclasses that detect gestures like double-taps will have to maintain their own state between touches
			GestureFinished();
			ResetGesture();
		}
		return;
	}
}

void UGestureRecognizerComponent::ReceiveTick(float DeltaTime)
{
	Super::ReceiveTick(DeltaTime);
	
	if (!PlayerInput->IsValidLowLevel())
	{
		// If not valid, try again to get it.
		
		
		APlayerController *Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (Controller->IsValidLowLevel())
		{
			PlayerInput = Controller->PlayerInput;
		}
		
		// If still not valid, return
		if (!PlayerInput->IsValidLowLevel())
			return;
	}
	ProcessTouches(PlayerInput->Touches, DeltaTime);
	DetectGestures(DeltaTime);
	PreviousTouchCount = CurrentTouchCount;
}

// ===========================================================================
// Detection Methods for subclasses to override

void UGestureRecognizerComponent::DetectGestures(float DeltaTime)
{
	
}

void UGestureRecognizerComponent::GestureFinished()
{
	
}

void UGestureRecognizerComponent::TouchBegan(int32 FingerIndex, FVector2D Position)
{
	
}

void UGestureRecognizerComponent::TouchMoved(int32 FingerIndex, FVector2D Position)
{
	
}

void UGestureRecognizerComponent::TouchEnded(int32 FingerIndex)
{
	
}

// ===========================================================================
// Private Methods

void UGestureRecognizerComponent::ResetGesture()
{
	CurrentTouchCount = PreviousTouchCount = MaxFingersThisGesture = 0;
	for (int32 Index = 0; Index < ARRAY_COUNT(TouchData); Index++)
	{
		TouchData[Index].TouchPoints.Empty();
		TouchData[Index].FirstTouchTime = 0;
		TouchData[Index].LatestTouchTime = 0;
		TouchData[Index].bIsTouchEnded = false;
		TouchData[Index].bIsTouchStarted = false;
	}
}

// Store the touches in easy to process format
void UGestureRecognizerComponent::UpdateStoredTouchData(const FVector (&Touches)[EKeys::NUM_TOUCH_KEYS])
{
	
	// No touches this or last frame, nothing to do, so skip out early
	if (CurrentTouchCount == 0 && PreviousTouchCount == 0) return;
	
	float now = GetWorld()->GetRealTimeSeconds(); // Real time, not game time, because should work on pause or time dilation
	
	if (CurrentTouchCount > 0 && PreviousTouchCount == 0)
	{
		FirstFingerTouchTime = now;
	}

	for (int32 Index = 0; Index < ARRAY_COUNT(TouchData); Index++)
	{
		
		// If it's a touch, store the position of the touch
		if (Index < CurrentTouchCount)
		{
			TouchData[Index].TouchPoints.Add(FVector2D(Touches[Index].X, Touches[Index].Y));
			
			// If it's a new touch, mark as started
			if (PreviousTouchCount < Index + 1 )
			{
				TouchData[Index].bIsTouchStarted = true;
				TouchBegan(Index, TouchData[Index].TouchPoints.Last());
			}
			else
			{
				TouchMoved(Index, TouchData[Index].TouchPoints.Last());
			}
		}
		// Not a touch, mark it as ended.
		else
		{
			TouchData[Index].bIsTouchEnded = true;
			TouchEnded(Index);
		}
	}
	LastFingerTouchTime = now;
}

void UGestureRecognizerComponent::CalculateTouchCount(const FVector (&Touches)[EKeys::NUM_TOUCH_KEYS])
{
	int32 TouchCount = 0;
	for (int32 TouchIndex = 0; TouchIndex < EKeys::NUM_TOUCH_KEYS; TouchIndex++)
	{
		if (Touches[TouchIndex].Z != 0)
		{
			TouchCount++;
		}
	}
	CurrentTouchCount = TouchCount;
	if (CurrentTouchCount > MaxFingersThisGesture)
	{
		MaxFingersThisGesture = CurrentTouchCount;
	}
}