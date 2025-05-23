// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RuntimeTransformer.h"
#include "TransformerActor.generated.h"

UENUM(BlueprintType)
enum class EGizmoPlacement : uint8
{
	GP_None					UMETA(DisplayName = "None"),
	GP_OnFirstSelection		UMETA(DisplayName = "On First Selection"),
	GP_OnLastSelection		UMETA(DisplayName = "On Last Selection"),
};

UCLASS()
class RUNTIMETRANSFORMER_API ATransformerActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATransformerActor();

private:

	//Gets the UFocusable Object. If ComponentBased, returns the UFocusable Component or nullptr (if it doesn't implement)
	// if ActorBased, returns the UFosuable Owner Actor or nullptr (if it doesn't implement)
	class UObject* GetUFocusable(class USceneComponent* Component) const;

	//Sets the Transform for a Given Component and calls the
	//Ufocusable transform function called if it implements the Interface
	void SetTransform(class USceneComponent* Component, const FTransform& Transform);

	//Called when the Component is added to the SelectedComponent List
	// Calls the IFocusableObject::Focus if the Component implements the UFocusable interface
	void Select(class USceneComponent* Component, bool* bImplementsUFocusable = nullptr);

	// Called when the Component is removed from the SelectedComponent List
	// Calls the IFocusableObject::Unfocus if the Component implements the UFocusable interface
	void Deselect(class USceneComponent* Component, bool* bImplementsUFocusable = nullptr);

	//Used to Filter unwanted things from a list of OutHits.
	void FilterHits(TArray<FHitResult>& outHits);

public:

	/*
	* This gets called everytime a Component / Actor is going to get added.
	* The default return is TRUE, but it can be overriden to check for additional things
	* (e.g. checking if it implements an interface, has some property, is child of a class, etc)

	* @param OwnerActor: The Actor owning the Component Selected
	* @param Component: The Component Selected (if it's an Actor Selected, this would be its RootComponent)

	* @return bool: Whether or not this Component should be added.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Runtime Transformer")
	bool ShouldSelect(AActor* OwnerActor, class USceneComponent* Component);

	//by default return true, override for custom logic
	virtual bool ShouldSelect_Implementation(AActor* OwnerActor, class USceneComponent* Component) { return true; }

	//Sets the Space of the Gizmo, whether its Local or World space.
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SetSpaceType(ESpaceType Type);

	/*
	 * Gets the Current Domain
	 * If it returns ETransformationDomain::TD_None, then that means
	 * there is no Transformation in Progress.
	*/
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	ETransformationDomain GetCurrentDomain(bool& TransformInProgress) const;


	/**
	 * Sets the Current Domain to NONE. (Transforming in Progress will become false)
	 * Should be called when we are done transforming with the Gizmo.
	*/
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void ClearDomain();

	//Gets the Start and End Points of the Mouse based on the Player Controller possessing this pawn
	// returns true if outStartPoint and outEndPoint were given a successful value
	bool CalculateMouseWorldPosition(float TraceDistance, FVector& outStartPoint, FVector& outEndPoint);

	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	 * This function only does the actual trace if there is a Player Controller Set
	 * @see SetPlayerController

	 * @param CollisionChannels - All the Channels to be considering during Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bAppendToList - If a selection happens, whether to append to the previously selected components or not
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	bool MouseTraceByObjectTypes(float TraceDistance
		, TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels
		, TArray<AActor*> IgnoredActors
		, bool bAppendToList = false
		, bool bTraceComplex = false);

	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	 * This function only does the actual trace if there is a Player Controller Set
	 * @see SetPlayerController

	 * @param TraceChannel - The Ray Collision Channel to be Considered in the Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bAppendToList - If a selection happens, whether to append to the previously selected components or not
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	bool MouseTraceByChannel(float TraceDistance
		, TEnumAsByte<ECollisionChannel> TraceChannel
		, TArray<AActor*> IgnoredActors
		, bool bAppendToList = false
		, bool bTraceComplex = false);

	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	 * This function only does the actual trace if there is a Player Controller Set
	 * @see SetPlayerController

	 * @param ProfileName - The Profile Name to be used during the Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bAppendToList - If a selection happens, whether to append to the previously selected components or not
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	bool MouseTraceByProfile(float TraceDistance
		, const FName& ProfileName
		, TArray<AActor*> IgnoredActors
		, bool bAppendToList = false
		, bool bTraceComplex = false);

	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	 * Note: This function does not Deselect the Objects selected if Trace doesn't select anything in any situation

	 * @param StartLocation - the starting Location of the trace, in World Space
	 * @param EndLocation - the ending location of the trace, in World Space
	 * @param CollisionChannels - All the Channels to be considering during Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bAppendToList - If a selection happens, whether to append to the previously selected components or not
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	bool TraceByObjectTypes(const FVector& StartLocation
		, const FVector& EndLocation
		, TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels
		, TArray<AActor*> IgnoredActors
		, bool bAppendToList = false
		, bool bTraceComplex = false);

	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	  * Note: This function does not Deselect the Objects selected if Trace doesn't select anything in any situation

	 * @param StartLocation - the starting Location of the trace, in World Space
	 * @param EndLocation - the ending location of the trace, in World Space
	 * @param TraceChannel - The Ray Collision Channel to be Considered in the Trace
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bAppendToList - If a selection happens, whether to append to the previously selected components or not
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	bool TraceByChannel(const FVector& StartLocation
		, const FVector& EndLocation
		, TEnumAsByte<ECollisionChannel> TraceChannel
		, TArray<AActor*> IgnoredActors
		, bool bAppendToList = false
		, bool bTraceComplex = false);


	/**
	 * If a Gizmo is Present, (i.e. there is a Selected Object), then
	 * this test will prioritize finding a Gizmo, even if it is behind an object.
	 * If there is not a Gizmo present, the first Object encountered will be automatically Selected.

	  * Note: This function does not Deselect the Objects selected if Trace doesn't select anything in any situation

	 * @param StartLocation - the starting Location of the trace, in World Space
	 * @param EndLocation - the ending location of the trace, in World Space
	 * @param ProfileName - The Profile Name to be used during the Trace
	 * @param bTraceComplex
	 * @param Ignored Actors	- The Actors to be Ignored during trace
	 * @param bAppendToList - If a selection happens, whether to append to the previously selected components or not
	 * @return bool Whether there was an Object traced successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	bool TraceByProfile(const FVector& StartLocation
                        , const FVector& EndLocation
                        , const FName& ProfileName
                        , TArray<AActor*> IgnoredActors
                        , bool bAppendToList = false
                        , bool bTraceComplex = false);

	// Update every Frame
	// Checks for Mouse Update
	virtual void Tick(float DeltaSeconds) override;

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	/**
	 * If the Gizmo is currently in a Valid Domain,
	 * it will transform the Selected Object(s) through a valid domain.
	 * The transform is calculated with the given Ray Origin and Ray Direction in World Space (usually the Mouse World Position & Mouse World Direction)

	 * This function should be called if NO Player Controller has been Set

	 * @param LookingVector - The looking direction of the player (e.g. Camera Forward Vector)
	 * @param RayOrigin - The origin point (world space) of the Ray (e.g. the Mouse Position in World Space)
	 * @param RayDirection - the direction of the ray (in world space) (e.g. the Mouse Direction in World Space)

	 * @returnval FTransform - The delta transform calculated (after any snapping)
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	FTransform UpdateTransform(const FVector& LookingVector
		, const FVector& RayOrigin, const FVector& RayDirection);

	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void ApplyDeltaTransform(const FTransform& DeltaTransform);

	/**
	 * Processes the OutHits generated by Tracing and Selects either a Gizmo (priority) or
	 * if no Gizmo is present in the trace, the first object hit is selected.
	 *
	 * This is already called by the RuntimeTransformer built-in Trace Functions,
	 * but can be called manually if you wish to provide your own list of Hit Results (e.g. tracing with different configuration/method)
	 *
	 * @param HitResults - a list of the FHitResults that were generated by LineTracing
	 * @param bAppendToList - If a selection happens, whether to append to the previously selected components or not
	*/
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	bool HandleTracedObjects(const TArray<FHitResult>& HitResults
		, bool bAppendToList = false);

	/*
	 * Called when the Gizmo State has changed (i.e. Domain has changed)
	 * @param GizmoType - the type of Gizmo that was changed (Translation, Rotation or Scale)
	 * @param bTransformInProgress - whether the Transform is currently in progress. This is basically a bool that evaluates to Domain != NONE
	 * @param Domain - The current domain that the Gizmo State changed to
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Runtime Transformer")
	void OnGizmoStateChanged(ETransformationType GizmoType, bool bTransformInProgress, ETransformationDomain Domain);

	virtual void OnGizmoStateChanged_Implementation(ETransformationType GizmoType, bool bTransformInProgress, ETransformationDomain Domain)
	{
		//this should be overriden for custom logic
	}

	/*
	 * Called when a new Component has been Selected (Focused)
	 * or has been unselected (unfocused).

	 * This should be used if there needs to be logic applied to
	 * objects that do not implement the UFocusable interface.

	 * @param Component - the Component selected/deselected
	 * @param bSelected - whether the given component was selected or unselected
	 * @param bImplementsUFocusable - whether the given Component implements UFocusable (Actor considered, if actors are traced)
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Runtime Transformer")
	void OnComponentSelectionChange(class USceneComponent* Component, bool bSelected, bool bImplementsUFocusable);

	virtual void OnComponentSelectionChange_Implementation(class USceneComponent* Component, bool bSelected, bool bImplementsUFocusable)
	{
		//This should be overriden for custom logic
	    if (UPrimitiveComponent* primitiveComp = Cast<UPrimitiveComponent>(Component))
	    {
            primitiveComp->SetRenderCustomDepth(bSelected);
	        if (primitiveComp->CustomDepthStencilValue == 0)
	        {
	            primitiveComp->CustomDepthStencilValue = 1;
	        }
	    }
	}

public:

	/**
	 * Whether to Set the System to work with Components (true)
	 * or to work with Actors (false)

	 @see bComponentBased
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SetComponentBased(bool bIsComponentBased);

	/**
	 * Whether to Set the System to Rotate Multiple Objects around their own axis (true)
	 * or to work rotate around where the Gizmo is at (false)

	 @see bRotateLocalAxis
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SetRotateOnLocalAxis(bool bRotateLocalAxis);

	/**
	 * Sets the Current Transformation (Translation, Rotation or Scale)
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SetTransformationType(ETransformationType TransformationType);

	/*
	 * Enables/Disables Snapping for a given Transformation
	 * Snapping Value for the Given Transformation MUST NOT be 0 for Snapping to work

	 @see SetSnappingValue
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SetSnappingEnabled(ETransformationType TransformationType, bool bSnappingEnabled);

	/*
	 * Sets a Snapping Value for a given Transformation
	 * Snapping Value MUST NOT be 0  and Snapping must be enabled for the given transformation for Snapping to work

	 @see SetSnappingEnabled
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SetSnappingValue(ETransformationType TransformationType, float SnappingValue);

	/*
	 * Gets the list of Selected Components.

	 @return outComponentList - the List of Currently Selected Components
	 @return outGizmoPlacedComponent - the Component in the list that currently has the Gizmo attached
	*/
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void GetSelectedComponents(TArray<class USceneComponent*>& outComponentList, class USceneComponent*& outGizmoPlacedComponent) const;

	TArray<class USceneComponent*> GetSelectedComponents() const;

	/*
	* Makes an exact copy of the Actors that are owners of the components and makes
	* a copy of them.

	* Don't spam this :)
	* @param bSelectNewClones - whether to add the new clones to the Selection
	* @param bAppendToList - If the New Clones are selected, whether to Append them to the List or Clear the previous Selections
	*/
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void CloneSelected(bool bSelectNewClones = true, bool bAppendToList = false);

protected:

	TArray<class USceneComponent*> CloneFromList(const TArray<class USceneComponent*>& ComponentList);

private:

	TArray<class USceneComponent*> CloneActors(const TArray<AActor*>& Actors);

	TArray<class USceneComponent*> CloneComponents(const TArray<class USceneComponent*>& Components);

public:
	/**
	 * Select Component adds a given Component to a list of components that will be used for the Runtime Transforms
	 * @param Component The component to add to the list.
	 * @param bAppendToList - If a selection happens, whether to append to the previously selected components or not
	*/
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SelectComponent(class USceneComponent* Component, bool bAppendToList = false);

	/**
	 * Select Actor adds the Actor's Root Component to a list of components that will be used for the Runtime Transforms
	 * @param Actor The Actor whose Root Component will be added to the list.
	 * @param bAppendToList - If a selection happens, whether to append to the previously selected components or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SelectActor(AActor* Actor, bool bAppendToList = false);

	/**
	 * Selects all the Components in given list.
	 * @see SelectComponent func
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SelectMultipleComponents(const TArray<class USceneComponent*>& Components
		, bool bAppendToList = false);

	/**
	 * Selects all the Root Components of the Actors in given list.
	 * @see SelectActor func
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void SelectMultipleActors(const TArray<AActor*>& Actors
		, bool bAppendToList = false);

	/**
	 * Deselects a given Component, if found on the list.
	 * @param Component the Component to deselect
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void DeselectComponent(class USceneComponent* Component);

	/**
	 * Deselects a given Actor's Root Component, if found on the list.
	 * @param Actor whose RootComponent to deselect
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	void DeselectActor(AActor* Actor);

	/*
	* Deselects all the Selected Components that are in the list.

	* @param bDestroyComponents - whether to Deselect all Components and Destroy them!

	* @return the list of components that were Deselected. (list will be empty if bDestroyComponents is true)
	*/
	UFUNCTION(BlueprintCallable, Category = "Runtime Transformer")
	TArray<class USceneComponent*> DeselectAll(bool bDestroyDeselected = false);

private:

	/*
	The core functionality, but can be called by Selection of Multiple objects
	so as to not call UpdateGizmo every time
	*/
	void AddComponent_Internal(TArray<class USceneComponent*>& OutComponentList
		, class USceneComponent* Component);

	/*
	The core functionality, but can be called by Selection of Multiple objects
	so as to not call UpdateGizmo every time
	*/
	void DeselectComponent_Internal(TArray<class USceneComponent*>& OutComponentList
		, class USceneComponent* Component);
	void DeselectComponentAtIndex_Internal(TArray<class USceneComponent*>& OutComponentList
		, int32 Index);
    class ABaseGizmo* CreateGizmo(ETransformationType transformationType);

    /**
	 * Creates / Replaces Gizmo with the Current Transformation.
	 * It destroys any current active gizmo to replace it.
	*/
	void SetGizmo();
    void ResetGizmo();

	/**
	 * Updates the Gizmo Placement (Position)
	 * Called when an object was selected, deselected
	*/
	void UpdateGizmoPlacement();

	//Gets the respective assigned class for a given TransformationType
	UClass* GetGizmoClass(ETransformationType TransformationType) const;

	//Resets the transform to all Zeros (including Scale)
	static void ResetDeltaTransform(FTransform& Transform);

	void SetDomain(ETransformationDomain Domain);

private:

	//The Current Space being used, whether it is Local or World.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	ESpaceType CurrentSpaceType;

	//The Transform Accumulated for Snapping
	FTransform AccumulatedDeltaTransform;

	/**
	 * GizmoClasses are variables that specified which Gizmo to spawn for each
	 * transformation. This can even be childs of classes that are already defined
	 * to allow the user to customize gizmo functionality
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gizmo", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ATranslationGizmo> TranslationGizmoClass;

	/**
	 * GizmoClasses are variables that specified which Gizmo to spawn for each
	 * transformation. This can even be childs of classes that are already defined
	 * to allow the user to customize gizmo functionality
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gizmo", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ARotationGizmo> RotationGizmoClass;

	/**
	 * GizmoClasses are variables that specified which Gizmo to spawn for each
	 * transformation. This can even be childs of classes that are already defined
	 * to allow the user to customize gizmo functionality
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gizmo", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AScaleGizmo> ScaleGizmoClass;

	UPROPERTY()
	TWeakObjectPtr<class ABaseGizmo> Gizmo;

    TArray<TWeakObjectPtr<ABaseGizmo>> GizmoActorPool;

	// Tell which Domain is Selected. If NONE, then that means that there is no Selected Objects, or
	// that the Gizmo has not been hit yet.
	ETransformationDomain CurrentDomain;

	//Tell where the Gizmo should be placed when multiple objects are selected
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	EGizmoPlacement GizmoPlacement;

	// Var that tells which is the Current Transformation taking place
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	ETransformationType CurrentTransformation;

	/**
	 * Array storing Selected Components. Although a quick O(1) removal is needed (like a Set),
	 * it is Crucial that we maintain the order of the elements as they were selected
	 */
	TArray<class USceneComponent*> SelectedComponents;

	/*
	* Map storing the Snap values for each transformation
	* bSnappingEnabled must be true AND, the value for the current transform MUST NOT be 0 for these values to take effect.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	TMap<ETransformationType, float> SnappingValues;

	/**
	* Whether Snapping an Object for each Transformation is enabled or not.
	* SnappingValue for each Transformation must also NOT be zero for it to work
	* (if, snapping value is 0 for a transformation, no snapping will take place)

	* @see SetSnappingValue function & SnappingValues Map var
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	TMap<ETransformationType, bool> SnappingEnabled;

	/**
	* Whether to Force Mobility on items that are not Moveable
	* if true, Mobility on Components will be changed to Moveable (WARNING: does not set it back to its original mobility!)
	* if false, no movement transformations will be attempted on Static/Stationary Components

	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	bool bForceMobility;

	/*
	 * This property only matters when multiple objects are selected.
	 * Whether multiple objects should rotate on their local axes (true) or on the axes the Gizmo is in (false)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	bool bRotateOnLocalAxis;

	/**
	 * Whether to Apply the Transforms to objects that Implement the UFocusable Interface.
	 * if True, the Transforms will be applied.
	 * if False, the Transforms will not be applied.

	 * IN BOTH Situations, the UFocusable Objects have IFocusable::OnNewDeltaTransformation called.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	bool bTransformUFocusableObjects;

	//Property that checks whether a CLICK on an already selected object should deselect the object or not.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	bool bToggleSelectedInMultiSelection;

	/*
	 * Property that checks whether Components are considered in trace
	 or the Actors are.
	 * This property affects how Cloning, Tracing is done and Interface checking is done
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
	bool bComponentBased;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime Transformations", meta = (AllowPrivateAccess = "true"))
    FName AttachSocketName = NAME_None;
};
