// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.


#include "TransformerActor.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

/* Gizmos */
#include "Gizmos/BaseGizmo.h"
#include "Gizmos/TranslationGizmo.h"
#include "Gizmos/RotationGizmo.h"
#include "Gizmos/ScaleGizmo.h"

/* Interface */
#include "FocusableObject.h"

// Sets default values
ATransformerActor::ATransformerActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GizmoPlacement			= EGizmoPlacement::GP_OnLastSelection;
	CurrentTransformation	= ETransformationType::TT_Translation;
	CurrentDomain			= ETransformationDomain::TD_None;
	CurrentSpaceType		= ESpaceType::ST_World;
	TranslationGizmoClass	= ATranslationGizmo::StaticClass();
	RotationGizmoClass		= ARotationGizmo::StaticClass();
	ScaleGizmoClass			= AScaleGizmo::StaticClass();

	ResetDeltaTransform(AccumulatedDeltaTransform);

	SetTransformationType(CurrentTransformation);
	SetSpaceType(CurrentSpaceType);

	bTransformUFocusableObjects = true;
	bRotateOnLocalAxis = false;
	bForceMobility = false;
	bToggleSelectedInMultiSelection = true;
	bComponentBased = false;
}

UObject* ATransformerActor::GetUFocusable(USceneComponent* Component) const
{
	if (!Component) return nullptr;

	if (bComponentBased)
	{
		return (Component->Implements<UFocusableObject>()) ? Component : nullptr;
	}

	if (AActor* ComponentOwner = Component->GetOwner())
	{
		return (ComponentOwner->Implements<UFocusableObject>()) ? ComponentOwner : nullptr;
	}

	return nullptr;
}

void ATransformerActor::SetTransform(USceneComponent* Component, const FTransform& Transform)
{
	if (!Component) return;
	if (UObject* focusableObject = GetUFocusable(Component))
	{
		IFocusableObject::Execute_OnNewTransformation(focusableObject, this, Component, Transform, bComponentBased);
		if (bTransformUFocusableObjects)
			Component->SetWorldTransform(Transform);
	}
	else
	{
	    Component->SetWorldTransform(Transform);
	}
}

void ATransformerActor::Select(USceneComponent* Component, bool* bImplementsUFocusable)
{
	UObject* focusableObject = GetUFocusable(Component);
	if (focusableObject)
		IFocusableObject::Execute_Focus(focusableObject, this, Component, bComponentBased);
	if (bImplementsUFocusable)
		*bImplementsUFocusable = !!focusableObject;
}

void ATransformerActor::Deselect(USceneComponent* Component, bool* bImplementsUFocusable)
{
	UObject* focusableObject = GetUFocusable(Component);
	if (focusableObject)
		IFocusableObject::Execute_Unfocus(focusableObject, this, Component, bComponentBased);
	if (bImplementsUFocusable)
		*bImplementsUFocusable = !!focusableObject;
}

void ATransformerActor::FilterHits(TArray<FHitResult>& outHits)
{

}

void ATransformerActor::SetSpaceType(ESpaceType Type)
{
	CurrentSpaceType = Type;
	SetGizmo();
}

ETransformationDomain ATransformerActor::GetCurrentDomain(bool& TransformInProgress) const
{
	TransformInProgress = (CurrentDomain != ETransformationDomain::TD_None);
	return CurrentDomain;
}

void ATransformerActor::ClearDomain()
{
	//Clear the Accumulated tranform when we stop Transforming
	ResetDeltaTransform(AccumulatedDeltaTransform);
	SetDomain(ETransformationDomain::TD_None);
}

bool ATransformerActor::CalculateMouseWorldPosition(float TraceDistance, FVector& outStartPoint, FVector& outEndPoint)
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0)/*Cast< APlayerController>(Controller)*/)
	{
		FVector worldLocation, worldDirection;
		if (PlayerController->DeprojectMousePositionToWorld(worldLocation, worldDirection))
		{
			outStartPoint = worldLocation;
			outEndPoint = worldLocation + (worldDirection * TraceDistance);
			return true;
		}
	}
	return false;
}

UClass* ATransformerActor::GetGizmoClass(ETransformationType TransformationType) const /* private */
{
	//Assign correct Gizmo Class depending on given Transformation
	switch (TransformationType)
	{
	case ETransformationType::TT_Translation:	return TranslationGizmoClass;
	case ETransformationType::TT_Rotation:		return RotationGizmoClass;
	case ETransformationType::TT_Scale:			return ScaleGizmoClass;
	default:									return nullptr;
	}
}

void ATransformerActor::ResetDeltaTransform(FTransform& Transform)
{
	Transform = FTransform();
	Transform.SetScale3D(FVector::ZeroVector);
}

void ATransformerActor::SetDomain(ETransformationDomain Domain)
{
	CurrentDomain = Domain;

	if (Gizmo.IsValid())
	{
		Gizmo->SetTransformProgressState(CurrentDomain != ETransformationDomain::TD_None, CurrentDomain);
	}
}

bool ATransformerActor::MouseTraceByObjectTypes(float TraceDistance
	, TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels
	, TArray<AActor*> IgnoredActors, bool bAppendToList, bool bTraceComplex)
{
	FVector start, end;
	bool bTraceSuccessful = false;
	if (CalculateMouseWorldPosition(TraceDistance, start, end))
	{
		bTraceSuccessful = TraceByObjectTypes(start, end, CollisionChannels, IgnoredActors, bAppendToList, bTraceComplex);
	}
	return bTraceSuccessful;
}

bool ATransformerActor::MouseTraceByChannel(float TraceDistance
	, TEnumAsByte<ECollisionChannel> TraceChannel, TArray<AActor*> IgnoredActors
	, bool bAppendToList, bool bTraceComplex)
{
	FVector start, end;
	bool bTraceSuccessful = false;
	if (CalculateMouseWorldPosition(TraceDistance, start, end))
	{
		bTraceSuccessful = TraceByChannel(start, end, TraceChannel, IgnoredActors, bAppendToList, bTraceComplex);
	}
	return false;
}

bool ATransformerActor::MouseTraceByProfile(float TraceDistance
	, const FName& ProfileName
	, TArray<AActor*> IgnoredActors
	, bool bAppendToList, bool bTraceComplex)
{
	FVector start, end;
	bool bTraceSuccessful = false;
	if (CalculateMouseWorldPosition(TraceDistance, start, end))
	{

		bTraceSuccessful = TraceByProfile(start, end, ProfileName, IgnoredActors, bAppendToList, bTraceComplex);
	}
	return bTraceSuccessful;
}

bool ATransformerActor::TraceByObjectTypes(const FVector& StartLocation
	, const FVector& EndLocation
	, TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels
	, TArray<AActor*> IgnoredActors
	, bool bAppendToList
	, bool bTraceComplex)
{
	if (UWorld* world = GetWorld())
	{
		FCollisionObjectQueryParams CollisionObjectQueryParams;
		FCollisionQueryParams CollisionQueryParams;
	    CollisionQueryParams.bTraceComplex = bTraceComplex;

		//Add All Given Collisions to the Array
		for (auto& cc : CollisionChannels)
			CollisionObjectQueryParams.AddObjectTypesToQuery(cc);

		CollisionQueryParams.AddIgnoredActors(IgnoredActors);

		TArray<FHitResult> OutHits;
		if (world->LineTraceMultiByObjectType(OutHits, StartLocation, EndLocation
			, CollisionObjectQueryParams, CollisionQueryParams))
		{
			FilterHits(OutHits);
			return HandleTracedObjects(OutHits, bAppendToList);
		}
	}
	return false;
}

bool ATransformerActor::TraceByChannel(const FVector& StartLocation
	, const FVector& EndLocation
	, TEnumAsByte<ECollisionChannel> TraceChannel
	, TArray<AActor*> IgnoredActors
	, bool bAppendToList
	, bool bTraceComplex)
{
	if (UWorld* world = GetWorld())
	{
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActors(IgnoredActors);
	    CollisionQueryParams.bTraceComplex = bTraceComplex;

	    FCollisionResponseParams responseParams;

		TArray<FHitResult> OutHits;
		if (world->LineTraceMultiByChannel(OutHits, StartLocation, EndLocation
			, TraceChannel, CollisionQueryParams, responseParams))
		{
			FilterHits(OutHits);
			return HandleTracedObjects(OutHits, bAppendToList);
		}
	}
	return false;
}

bool ATransformerActor::TraceByProfile(const FVector& StartLocation
	, const FVector& EndLocation
	, const FName& ProfileName, TArray<AActor*> IgnoredActors
	, bool bAppendToList
	, bool bTraceComplex)
{
	if (UWorld* world = GetWorld())
	{
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActors(IgnoredActors);
	    CollisionQueryParams.bTraceComplex = bTraceComplex;

		TArray<FHitResult> OutHits;
		if (world->LineTraceMultiByProfile(OutHits, StartLocation, EndLocation
			, ProfileName, CollisionQueryParams))
		{
			FilterHits(OutHits);
			return HandleTracedObjects(OutHits, bAppendToList);
		}
	}
	return false;
}

void ATransformerActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Gizmo.IsValid()) return;

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0) /*Cast< APlayerController>(Controller)*/)
	{
		FVector worldLocation, worldDirection;
		if (PlayerController->IsLocalController() && PlayerController->PlayerCameraManager)
		{
			if (PlayerController->DeprojectMousePositionToWorld(worldLocation, worldDirection))
			{
				UpdateTransform(PlayerController->PlayerCameraManager->GetActorForwardVector(), worldLocation, worldDirection);
			}
		}
	}

	//Only consider Local View
	if (APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (LocalPlayerController->PlayerCameraManager)
		{
			Gizmo->ScaleGizmoScene(LocalPlayerController->PlayerCameraManager->GetCameraLocation()
				, LocalPlayerController->PlayerCameraManager->GetActorForwardVector()
				, LocalPlayerController->PlayerCameraManager->GetFOVAngle());
		}
	}

	Gizmo->UpdateGizmoSpace(CurrentSpaceType); //ToDo: change when this is called to improve performance when a gizmo is there without doing anything
}

void ATransformerActor::BeginPlay()
{
    Super::BeginPlay();

    for (auto it = GizmoActorPool.CreateIterator(); it; ++it)
    {
        if (it->IsValid())
        {
            (*it)->Destroy();
            it.RemoveCurrentSwap();
        }
    }

    TArray TransformationTypes = {
        ETransformationType::TT_Translation,
        ETransformationType::TT_Rotation,
        ETransformationType::TT_Scale
    };

    GizmoActorPool.SetNum(3);
    int i = 0;
    for (ETransformationType& tt : TransformationTypes)
    {
        ABaseGizmo* gizmo = CreateGizmo(tt);
        gizmo->SetActorHiddenInGame(true);
        gizmo->SetActorEnableCollision(false);
        gizmo->SetActorTickEnabled(false);
        ensureMsgf(gizmo, TEXT("Gizmo of type %s could not be created!"), *StaticEnum<ETransformationType>()->GetNameStringByValue(static_cast<int64>(tt)));
        if (gizmo)
        {
            GizmoActorPool[i] = (gizmo);
        }
        ++i;
        return;
    }
}

void ATransformerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    for (TWeakObjectPtr gizmo : GizmoActorPool)
    {
        if (gizmo.IsValid())
        {
            gizmo->Destroy();
        }
    }
}

#if WITH_EDITOR
void ATransformerActor::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = PropertyChangedEvent.GetPropertyName();
    FString PropertyNameString = PropertyName.ToString();
    if (PropertyName != GET_MEMBER_NAME_CHECKED(ATransformerActor, Gizmo) && PropertyNameString.StartsWith(TEXT("Current"), ESearchCase::IgnoreCase))
    {
        UpdateGizmoPlacement();
    }
}
#endif

FTransform ATransformerActor::UpdateTransform(const FVector& LookingVector
                                             , const FVector& RayOrigin
                                             , const FVector& RayDirection)
{
	FTransform deltaTransform;
	deltaTransform.SetScale3D(FVector::ZeroVector);

	if (!Gizmo.IsValid() || CurrentDomain == ETransformationDomain::TD_None)
		return deltaTransform;

	FVector rayEnd = RayOrigin + 1'000'000'00 * RayDirection;

	FTransform calcDeltaTransform = Gizmo->GetDeltaTransform(LookingVector, RayOrigin, rayEnd, CurrentDomain);

	//The delta transform we are actually going to apply (same if there is no Snapping taking place)
	deltaTransform = calcDeltaTransform;

	/* SNAPPING LOGIC */
	bool* snappingEnabled = SnappingEnabled.Find(CurrentTransformation);
	float* snappingValue = SnappingValues.Find(CurrentTransformation);

	if (snappingEnabled && *snappingEnabled && snappingValue)
			deltaTransform = Gizmo->GetSnappedTransform(AccumulatedDeltaTransform
				, calcDeltaTransform, CurrentDomain, *snappingValue);
				//GetSnapped Transform Modifies Accumulated Delta Transform by how much Snapping Occurred

	ApplyDeltaTransform(deltaTransform);
	return deltaTransform;
}

void ATransformerActor::ApplyDeltaTransform(const FTransform& DeltaTransform)
{
	bool* snappingEnabled = SnappingEnabled.Find(CurrentTransformation);
	float* snappingValue = SnappingValues.Find(CurrentTransformation);

	for (auto& sc : SelectedComponents)
	{
		if (!sc) continue;
		if (bForceMobility || sc->Mobility == EComponentMobility::Type::Movable)
		{
			const FTransform& componentTransform = sc->GetComponentTransform();

			FQuat deltaRotation = DeltaTransform.GetRotation();

			FVector deltaLocation = componentTransform.GetLocation()
				- Gizmo->GetActorLocation();

			//DeltaScale is Unrotated Scale to Get Local Scale since World Scale is not supported
			FVector deltaScale = componentTransform.GetRotation()
				.UnrotateVector(DeltaTransform.GetScale3D());


			if (false == bRotateOnLocalAxis)
				deltaLocation = deltaRotation.RotateVector(deltaLocation);

			FTransform newTransform(
				deltaRotation * componentTransform.GetRotation(),
				//adding Gizmo Location + prevDeltaLocation
				// (i.e. location from Gizmo to Object after optional Rotating)
				// + deltaTransform Location Offset
				deltaLocation + Gizmo->GetActorLocation() + DeltaTransform.GetLocation(),
				deltaScale + componentTransform.GetScale3D());


			/* SNAPPING LOGIC PER COMPONENT */
			if (snappingEnabled && *snappingEnabled && snappingValue)
				newTransform = Gizmo->GetSnappedTransformPerComponent(componentTransform
					, newTransform, CurrentDomain, *snappingValue);

			sc->SetMobility(EComponentMobility::Type::Movable);
			SetTransform(sc, newTransform);
		}
		else
		{
			UE_LOG(LogRuntimeTransformer, Warning, TEXT("Transform will not affect Component [%s] as it is NOT Moveable!"), *sc->GetName());
		}

	}
}

bool ATransformerActor::HandleTracedObjects(const TArray<FHitResult>& HitResults, bool bAppendToList)
{
	//Assign as None just in case we don't hit Any Gizmos
	ClearDomain();

	//Search for our Gizmo (if Valid) First before Selecting any item
	if (Gizmo.IsValid())
	{
		for (auto& hitResult : HitResults)
		{
			if (Gizmo == hitResult.GetActor()) //check if it's OUR gizmo
			{
				//Check which Domain of Gizmo was Hit from the Test
				if (USceneComponent* componentHit = Cast<USceneComponent>(hitResult.Component))
				{
					SetDomain(Gizmo->GetTransformationDomain(componentHit));
					if (CurrentDomain != ETransformationDomain::TD_None)
					{
						Gizmo->SetTransformProgressState(true, CurrentDomain);
						return true; //finish only if the component actually has a domain, else continue
					}
				}
			}
		}
	}

	for (auto& hits : HitResults)
	{
		if (Cast<ABaseGizmo>(hits.GetActor()))
			continue; //ignore other Gizmos.

		if (bComponentBased)
			SelectComponent(Cast<USceneComponent>(hits.GetComponent()), bAppendToList);
		else
			SelectActor(hits.GetActor(), bAppendToList);

		return true; //don't process more!
	}

	return false;
}

void ATransformerActor::SetComponentBased(bool bIsComponentBased)
{
	auto selectedComponents = DeselectAll();
	bComponentBased = bIsComponentBased;
	if(bComponentBased)
		SelectMultipleComponents(selectedComponents, false);
	else
	{
		TArray<AActor*> actors;
		for (auto& c : selectedComponents)
			actors.Add(c->GetOwner());
		SelectMultipleActors(actors, false);
	}

}

void ATransformerActor::SetRotateOnLocalAxis(bool bRotateLocalAxis)
{
	bRotateOnLocalAxis = bRotateLocalAxis;
}

void ATransformerActor::SetTransformationType(ETransformationType TransformationType)
{
	//Don't continue if these are the same.
	if (CurrentTransformation == TransformationType) return;

	if (TransformationType == ETransformationType::TT_NoTransform)
    {
        UE_LOG(LogRuntimeTransformer, Warning, TEXT("Setting Transformation Type to None!"));
    }


	CurrentTransformation = TransformationType;

	//Clear the Accumulated tranform when we have a new Transformation
	ResetDeltaTransform(AccumulatedDeltaTransform);

	UpdateGizmoPlacement();
}

void ATransformerActor::SetSnappingEnabled(ETransformationType TransformationType, bool bSnappingEnabled)
{
	SnappingEnabled.Add(TransformationType, bSnappingEnabled);
}

void ATransformerActor::SetSnappingValue(ETransformationType TransformationType, float SnappingValue)
{
	SnappingValues.Add(TransformationType, SnappingValue);
}

void ATransformerActor::GetSelectedComponents(TArray<class USceneComponent*>& outComponentList
	, USceneComponent*& outGizmoPlacedComponent) const
{
	outComponentList = SelectedComponents;
	if (Gizmo.IsValid())
		outGizmoPlacedComponent = Gizmo->GetParentComponent();
}

TArray<USceneComponent*> ATransformerActor::GetSelectedComponents() const
{
	return SelectedComponents;
}

void ATransformerActor::CloneSelected(bool bSelectNewClones
	, bool bAppendToList)
{
	if (GetLocalRole() < ROLE_Authority)
    {
        UE_LOG(LogRuntimeTransformer, Warning, TEXT("Cloning in a Non-Authority! Please use the Clone RPCs instead"));
    }


	auto CloneComponents = CloneFromList(SelectedComponents);

	if (bSelectNewClones)
		SelectMultipleComponents(CloneComponents, bAppendToList);
}

TArray<class USceneComponent*> ATransformerActor::CloneFromList(const TArray<USceneComponent*>& ComponentList)
{

	TArray<class USceneComponent*> outClones;
	if (bComponentBased)
	{
		TArray<USceneComponent*> Components;

		for (auto& i : ComponentList)
		{
			if (i) Components.Add(i);
		}
		outClones = CloneComponents(Components);
	}
	else
	{
		TArray<AActor*> Actors;
		for (auto& i : ComponentList)
		{
			if (i)
				Actors.Add(i->GetOwner());
		}
		outClones = CloneActors(Actors);
	}

	if (CurrentDomain != ETransformationDomain::TD_None && Gizmo.IsValid())
		Gizmo->SetTransformProgressState(true, CurrentDomain);

	return outClones;
}

TArray<class USceneComponent*> ATransformerActor::CloneActors(const TArray<AActor*>& Actors)
{
	TArray<class USceneComponent*> outClones;

	UWorld* world = GetWorld();
	if (!world) return outClones;

	TSet<AActor*>	actorsProcessed;
	TArray<AActor*> actorsToSelect;
	for (auto& templateActor : Actors)
	{
		if (!templateActor) continue;
		bool bAlreadyProcessed;
		actorsProcessed.Add(templateActor, &bAlreadyProcessed);
		if (bAlreadyProcessed) continue;

		FTransform spawnTransform;
		FActorSpawnParameters spawnParams;

		spawnParams.Template = templateActor;
		if (templateActor)
			templateActor->bNetStartup = false;

		if (AActor* actor = world->SpawnActor(templateActor->GetClass()
			, &spawnTransform, spawnParams))
		{
			outClones.Add(actor->GetRootComponent());
		}
	}
	return outClones;
}

TArray<class USceneComponent*> ATransformerActor::CloneComponents(const TArray<class USceneComponent*>& Components)
{
	TArray<class USceneComponent*> outClones;

	UWorld* world = GetWorld();
	if (!world) return outClones;

	TMap<USceneComponent*, USceneComponent*> OcCc; //Original component - Clone component
	TMap<USceneComponent*, USceneComponent*> CcOp; //Clone component - Original parent

	//clone components phase
	for (auto& templateComponent : Components)
	{
		if (!templateComponent) continue;
		AActor* owner = templateComponent->GetOwner();
		if (!owner) continue;

		if (USceneComponent* clone = Cast<USceneComponent>(
			StaticDuplicateObject(templateComponent, owner)))
		{
			PostCreateBlueprintComponent(clone);
			clone->OnComponentCreated();

			clone->RegisterComponent();
			clone->SetRelativeTransform(templateComponent->GetRelativeTransform());

			outClones.Add(clone);
			//Add to these two maps for reparenting in next phase
			OcCc.Add(templateComponent, clone); //Original component - Clone component

			if (templateComponent == owner->GetRootComponent())
				CcOp.Add(clone, owner->GetRootComponent()); //this will cause a loop in the maps, so we must check for this!
			else
				CcOp.Add(clone, templateComponent->GetAttachParent()); //Clone component - Original parent
		}
	}

	TArray<USceneComponent*> componentsToSelect;
	//reparenting phase
	FAttachmentTransformRules attachmentRule(EAttachmentRule::KeepWorld, false);
	for (auto& cp : CcOp)
	{
		//original parent
		USceneComponent* parent = cp.Value;

		AActor* actorOwner = cp.Value->GetOwner();

		//find if we cloned the original parent
		USceneComponent** cloneParent = OcCc.Find(parent);

		if (cloneParent)
		{
			if (*cloneParent != cp.Key) //make sure comp is not its own parent
				parent = *cloneParent;
		}
		else
		{
			//couldn't find its parent, so find the parent of the parent and see if it's in the list.
			//repeat until found or root is reached
			while (1)
			{
				//if parent is root, then no need to find parents above it. (none)
				//attach to original parent, since there are no cloned parents.
				if (parent == actorOwner->GetRootComponent())
				{
					parent = cp.Value;
					break;
				}

				//check if parents have been cloned
				cloneParent = OcCc.Find(parent->GetAttachParent());
				if (cloneParent)
				{
					//attach to cloned parent if found
					parent = *cloneParent;
					break;
				}
				parent = parent->GetAttachParent(); //move up in the hierarchy
			}
		}

		cp.Key->AttachToComponent(parent, attachmentRule);

		//Selecting childs and parents can cause weird issues
		// so only select the topmost clones (those that do not have cloned parents!)
		//only select those that have an "original parent".
		//if ((parent == cp.Value
		//	|| parent == actorOwner->GetRootComponent()))
		//	//check if the parent of the cloned is original (means it's topmost)
		//	outParents.Add(cp.Key);
	}

	return outClones;
}

void ATransformerActor::SelectComponent(class USceneComponent* Component, bool bAppendToList)
{
	if (!Component) return;

	if (ShouldSelect(Component->GetOwner(), Component))
	{
		if (bAppendToList == false)
		{
			DeselectAll();
		}
		AddComponent_Internal(SelectedComponents, Component);
		UpdateGizmoPlacement();
	}
}

void ATransformerActor::SelectActor(AActor* Actor
	, bool bAppendToList)
{
	if (!Actor) return;

	if (ShouldSelect(Actor, Actor->GetRootComponent()))
	{
		if (false == bAppendToList)
			DeselectAll();
		AddComponent_Internal(SelectedComponents, Actor->GetRootComponent());
		UpdateGizmoPlacement();
	}
}

void ATransformerActor::SelectMultipleComponents(const TArray<USceneComponent*>& Components
	, bool bAppendToList)
{
	bool bValidList = false;

	for (auto& c : Components)
	{
		if (!c) continue;
		if (!ShouldSelect(c->GetOwner(), c)) continue;

		if (false == bAppendToList)
		{
			DeselectAll();
			bAppendToList = true;
			//only run once. This is not place outside in case a list is empty or contains only invalid components
		}
		bValidList = true;
		AddComponent_Internal(SelectedComponents, c);
	}

	if(bValidList) UpdateGizmoPlacement();
}

void ATransformerActor::SelectMultipleActors(const TArray<AActor*>& Actors
	, bool bAppendToList)
{
	bool bValidList = false;
	for (auto& a : Actors)
	{
		if (!a) continue;
		if (!ShouldSelect(a, a->GetRootComponent())) continue;

		if (false == bAppendToList)
		{
			DeselectAll();
			bAppendToList = true;
			//only run once. This is not place outside in case a list is empty or contains only invalid components
		}

		bValidList = true;
		AddComponent_Internal(SelectedComponents, a->GetRootComponent());
	}
	if(bValidList) UpdateGizmoPlacement();
}

void ATransformerActor::DeselectComponent(USceneComponent* Component)
{
	if (!Component) return;
	DeselectComponent_Internal(SelectedComponents, Component);
	UpdateGizmoPlacement();
}

void ATransformerActor::DeselectActor(AActor* Actor)
{
	if (Actor)
		DeselectComponent(Actor->GetRootComponent());
}

TArray<USceneComponent*> ATransformerActor::DeselectAll(bool bDestroyDeselected)
{
	TArray<USceneComponent*> componentsToDeselect = SelectedComponents;
	for (auto& i : componentsToDeselect)
		DeselectComponent(i);
		//calling internal so as not to modify SelectedComponents until the last bit!
	SelectedComponents.Empty();
	UpdateGizmoPlacement();

	if (bDestroyDeselected)
	{
		for (auto& c : componentsToDeselect)
		{
			if (!IsValid(c)) continue; //a component that was in the same actor destroyed will be pending kill
			if (AActor* actor = c->GetOwner())
			{
				//We destroy the actor if no components are left to destroy, or the system is currently ActorBased
				if (bComponentBased && actor->GetComponents().Num() > 1)
					c->DestroyComponent(true);
				else
					actor->Destroy();
			}
		}
	}

	return componentsToDeselect;
}

void ATransformerActor::AddComponent_Internal(TArray<USceneComponent*>& OutComponentList
	, USceneComponent* Component)
{
	//if (!Component) return; //assumes that previous have checked, since this is Internal.

	int32 Index = OutComponentList.Find(Component);

	if (INDEX_NONE == Index) //Component is not in list
	{
		OutComponentList.Emplace(Component);
		bool bImplementsInterface;
		Select(OutComponentList.Last(), &bImplementsInterface);
		OnComponentSelectionChange(Component, true, bImplementsInterface);
	}
	else if (bToggleSelectedInMultiSelection)
		DeselectComponentAtIndex_Internal(OutComponentList, Index);
}

void ATransformerActor::DeselectComponent_Internal(TArray<USceneComponent*>& OutComponentList
	, USceneComponent* Component)
{
	//if (!Component) return; //assumes that previous have checked, since this is Internal.

	int32 Index = OutComponentList.Find(Component);
	if (INDEX_NONE != Index)
		DeselectComponentAtIndex_Internal(OutComponentList, Index);
}

void ATransformerActor::DeselectComponentAtIndex_Internal(
	TArray<USceneComponent*>& OutComponentList
	, int32 Index)
{
	//if (!Component) return; //assumes that previous have checked, since this is Internal.
	if (OutComponentList.IsValidIndex(Index))
	{
		USceneComponent* Component = OutComponentList[Index];
		bool bImplementsInterface;
		Deselect(Component, &bImplementsInterface);
		OutComponentList.RemoveAt(Index);
		OnComponentSelectionChange(Component, false, bImplementsInterface);
	}

}

ABaseGizmo* ATransformerActor::CreateGizmo(ETransformationType transformationType)
{
    ABaseGizmo* gizmo = nullptr;
    if (UWorld* world = GetWorld())
    {
        if (UClass* GizmoClass = GetGizmoClass(transformationType))
        {
            gizmo = Cast<ABaseGizmo>(world->SpawnActor(GizmoClass));
            gizmo->OnGizmoStateChange.AddDynamic(this, &ATransformerActor::OnGizmoStateChanged);
        }
    }
    return gizmo;
}

void ATransformerActor::SetGizmo()
{

	//If there are selected components, then we see whether we need to create a new gizmo.
	if (SelectedComponents.Num() > 0)
	{
		bool bCreateGizmo = true;
		if (Gizmo.IsValid())
		{
			if (CurrentTransformation == Gizmo->GetGizmoType())
			{
				bCreateGizmo = false; // do not create gizmo if there is already a matching gizmo
			}
			else
			{
				// Destroy the current gizmo as the transformation types do not match
			    ResetGizmo();
			}
		}

		if (bCreateGizmo)
		{
		    int32 index = static_cast<int32>(CurrentTransformation);
		    if (GizmoActorPool.IsValidIndex(index) && GizmoActorPool[index].IsValid())
            {
                Gizmo = GizmoActorPool[index];
		        Gizmo->SetActorHiddenInGame(false);
		        Gizmo->SetActorTickEnabled(true);
		        Gizmo->SetActorEnableCollision(true);
            }
            else
            {
                Gizmo = CreateGizmo(CurrentTransformation);
                GizmoActorPool.SetNum(index + 1);
                GizmoActorPool[index] = Gizmo;
            }
		}
	}
	//Since there are no selected components, we must destroy any gizmos present
	else
	{
		ResetGizmo();
	}
}

void ATransformerActor::ResetGizmo()
{
    if (Gizmo.IsValid())
    {
        Gizmo->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        Gizmo->SetActorHiddenInGame(true);
        Gizmo->SetActorTickEnabled(false);
        Gizmo->SetActorEnableCollision(false);
        Gizmo.Reset();
    }
}

void ATransformerActor::UpdateGizmoPlacement()
{
	SetGizmo();
	//means that there are no active gizmos (no selections) so nothing to do in this func
	if (!Gizmo.IsValid()) return;

	USceneComponent* ComponentToAttachTo = nullptr;

	switch (GizmoPlacement)
	{
	case EGizmoPlacement::GP_OnFirstSelection:
		ComponentToAttachTo = SelectedComponents[0]; break;
	case EGizmoPlacement::GP_OnLastSelection:
		ComponentToAttachTo = SelectedComponents.Last(); break;
	case EGizmoPlacement::GP_None:
	    UE_LOG(LogRuntimeTransformer, Warning, TEXT("Gizmo Placement is None! setting to last selection"));
	    ComponentToAttachTo = SelectedComponents.Last(); break;
	}

	if (ComponentToAttachTo)
	{
	    FName socketToAttach = AttachSocketName.IsNone() == false && ComponentToAttachTo->DoesSocketExist(AttachSocketName) ? AttachSocketName : NAME_None;
		Gizmo->AttachToComponent(ComponentToAttachTo, FAttachmentTransformRules::SnapToTargetIncludingScale, socketToAttach);
	}
	else
	{
	//	Gizmo->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}

	Gizmo->UpdateGizmoSpace(CurrentSpaceType);
}

#undef RTT_LOG
