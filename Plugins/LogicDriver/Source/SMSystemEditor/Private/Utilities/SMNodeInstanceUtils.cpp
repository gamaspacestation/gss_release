// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "Utilities/SMNodeInstanceUtils.h"
#include "Graph/Nodes/SMGraphNode_Base.h"
#include "Configuration/SMEditorStyle.h"
#include "SMBlueprintEditorUtils.h"
#include "SMPropertyUtils.h"

#include "SMUtils.h"

#include "IAssetTools.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_CallFunction.h"
#include "Widgets/Text/STextBlock.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "EdGraphSchema_K2.h"
#include "ISinglePropertyView.h"
#include "PropertyHandle.h"
#include "SMUnrealTypeDefs.h"

#define LOCTEXT_NAMESPACE "SMNodeInstanceUtils"

FString FSMNodeInstanceUtils::GetNodeDisplayName(const USMNodeInstance* InNodeInstance)
{
	check(InNodeInstance);
	return FSMBlueprintEditorUtils::GetProjectEditorSettings()->bRestrictInvalidCharacters ?
	FSMBlueprintEditorUtils::GetSafeStateName(InNodeInstance->GetNodeDisplayName()) : InNodeInstance->GetNodeDisplayName();
}

FText FSMNodeInstanceUtils::GetNodeDescriptionText(const USMNodeInstance* InNodeInstance)
{
	check(InNodeInstance);
	return InNodeInstance->GetNodeDescriptionText();
}

FText FSMNodeInstanceUtils::GetNodeCategory(const USMNodeInstance* InNodeInstance)
{
	check(InNodeInstance);
	const FSMNodeDescription& Description = InNodeInstance->GetNodeDescription();
	if (!Description.Category.IsEmpty())
	{
		return Description.Category;
	}

	return InNodeInstance->GetClass()->GetMetaDataText(TEXT("Category"));
}

bool FSMNodeInstanceUtils::IsWidgetChildOf(TSharedPtr<SWidget> Parent, TSharedPtr<SWidget> PossibleChild)
{
	FChildren* Children = Parent->GetChildren();
	for (int32 i = 0; i < Children->Num(); ++i)
	{
		TSharedRef<SWidget> Child = Children->GetChildAt(i);
		if (Child == PossibleChild)
		{
			return true;
		}
		return IsWidgetChildOf(Child, PossibleChild);
	}

	return false;
}

FText FSMNodeInstanceUtils::CreateNodeClassTextSummary(const USMNodeInstance* NodeInstance)
{
	check(NodeInstance);

	const FString Name = GetNodeDisplayName(NodeInstance);
	const FText Description = GetNodeDescriptionText(NodeInstance);

	const FText TextFormat = FText::FromString(Description.IsEmpty() ? "{0}" : "{0} - {1}");
	return FText::Format(TextFormat, FText::FromString(Name), Description);
}

TSharedPtr<SWidget> FSMNodeInstanceUtils::CreateNodeClassWidgetDisplay(const USMNodeInstance* NodeInstance)
{
	check(NodeInstance);
	
	const FSMNodeDescription& Description = NodeInstance->GetNodeDescription();

	FString ClassName = NodeInstance->GetClass()->GetName();
	ClassName.RemoveFromEnd(TEXT("_C"));
	const FString Name = Description.Name.IsNone() ? ClassName : Description.Name.ToString();
	
	const FText TextFormat = FText::FromString(Description.Description.IsEmpty() ? "{0}" : "{0} - {1}");
	const FText NodeClassSummaryText = CreateNodeClassTextSummary(NodeInstance);
	
	return SNew(SOverlay)
	+ SOverlay::Slot()
	[
		SNew(SBorder)
		.BorderImage(FSMUnrealAppStyle::Get().GetBrush("Graph.Node.TitleBackground"))
		.BorderBackgroundColor(FLinearColor(0.4f, 0.4f, 0.4f, 0.4f))
	]
	+ SOverlay::Slot()
	.VAlign(VAlign_Center)
	.Padding(FMargin(6,4))
	[
		SNew(STextBlock)
			.Text(NodeClassSummaryText)
			.TextStyle(FSMUnrealAppStyle::Get(), TEXT("NormalText"))
			.ColorAndOpacity(FLinearColor::White)
	];
}

const FGuid& FSMNodeInstanceUtils::SetGraphPropertyFromProperty(FSMGraphProperty_Base& GraphProperty,
	FProperty* Property, USMNodeInstance* NodeInstance, int32 Index, bool bSetGuid, bool bUseTemplateInGuid, bool bUseTempNativeGuid)
{
	check(NodeInstance)
	check(Property);
	
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	
	GraphProperty.bIsInArray = Property->IsA<FArrayProperty>() || Property->GetOwnerProperty()->IsA<FArrayProperty>();
	
	GraphProperty.VariableName = Property->GetFName();
	GraphProperty.MemberReference.SetFromField<FProperty>(Property, false);
	K2Schema->ConvertPropertyToPinType(Property, GraphProperty.VariableType);

	// TemplateGuid is used to calculate final guid.
	GraphProperty.SetTemplateGuid(NodeInstance->GetTemplateGuid());

	if (!bSetGuid)
	{
		return GraphProperty.GetGuid();
	}
	
	if (GraphProperty.MemberReference.GetMemberGuid().IsValid())
	{
		// Blueprint variable
		return GraphProperty.SetGuid(GraphProperty.MemberReference.GetMemberGuid(), Index, bUseTemplateInGuid);
	}

	// Search string Taken from FMemberReference::GetReferenceSearchString of engine CL 17816129.
	auto GetTempNativeSearchString = [&] (UClass* InFieldOwner)
	{
		const FGuid MemberGuid = GraphProperty.MemberReference.GetMemberGuid();
		const FName MemberName = GraphProperty.MemberReference.GetMemberName();
		if (!GraphProperty.MemberReference.IsLocalScope())
		{
			if (InFieldOwner)
			{
				if (MemberGuid.IsValid())
				{
					return FString::Printf(TEXT("Nodes(VariableReference(MemberName=+\"%s\" && MemberGuid(A=%i && B=%i && C=%i && D=%i)) || Name=\"(%s)\") || Pins(Binding=\"%s\") || Binding=\"%s\""), *MemberName.ToString(), MemberGuid.A, MemberGuid.B, MemberGuid.C, MemberGuid.D, *MemberName.ToString(), *MemberName.ToString(), *MemberName.ToString());
				}
				else
				{
					FString ExportMemberParentName = InFieldOwner->GetClass()->GetName();
					ExportMemberParentName.AppendChar('\'');
					ExportMemberParentName += InFieldOwner->GetAuthoritativeClass()->GetPathName();
					ExportMemberParentName.AppendChar('\'');

					return FString::Printf(TEXT("Nodes(VariableReference(MemberName=+\"%s\" && (MemberParent=\"%s\" || bSelfContext=true) ) || Name=\"(%s)\") || Pins(Binding=\"%s\") || Binding=\"%s\""), *MemberName.ToString(), *ExportMemberParentName, *MemberName.ToString(), *MemberName.ToString(), *MemberName.ToString());
				}
			}
			else if (MemberGuid.IsValid())
			{
				return FString::Printf(TEXT("Nodes(VariableReference(MemberName=+\"%s\" && MemberGuid(A=%i && B=%i && C=%i && D=%i)) || Name=\"(%s)\") || Pins(Binding=\"%s\") || Binding=\"%s\""), *MemberName.ToString(), MemberGuid.A, MemberGuid.B, MemberGuid.C, MemberGuid.D, *MemberName.ToString(), *MemberName.ToString(), *MemberName.ToString());
			}
			else
			{
				return FString::Printf(TEXT("Nodes(VariableReference(MemberName=+\"%s\") || Name=\"(%s)\") || Pins(Binding=\"%s\") || Binding=\"%s\""), *MemberName.ToString(), *MemberName.ToString(), *MemberName.ToString(), *MemberName.ToString());
			}
		}
		else
		{
			return FString::Printf(TEXT("Nodes(VariableReference((MemberName=+\"%s\" && MemberScope=+\"%s\"))) || Binding=\"%s\""), *MemberName.ToString(), *GraphProperty.MemberReference.GetMemberScopeName(), *MemberName.ToString());
		}
	};

	// Previous search string, from 4.27 & below. This is what is used currently.
	auto GetNativeSearchString = [&](UClass* InFieldOwner)
	{
		const FGuid MemberGuid = GraphProperty.MemberReference.GetMemberGuid();
		const FName MemberName = GraphProperty.MemberReference.GetMemberName();
		if (!GraphProperty.MemberReference.IsLocalScope())
		{
			if (InFieldOwner)
			{
				if (MemberGuid.IsValid())
				{
					return FString::Printf(TEXT("Nodes(VariableReference(MemberName=+\"%s\" && MemberGuid(A=%i && B=%i && C=%i && D=%i) ))"), *MemberName.ToString(), MemberGuid.A, MemberGuid.B, MemberGuid.C, MemberGuid.D);
				}
				else
				{
					FString ExportMemberParentName = InFieldOwner->GetClass()->GetName();
					ExportMemberParentName.AppendChar('\'');
					ExportMemberParentName += InFieldOwner->GetAuthoritativeClass()->GetPathName();
					ExportMemberParentName.AppendChar('\'');

					return FString::Printf(TEXT("Nodes(VariableReference(MemberName=+\"%s\" && (MemberParent=\"%s\" || bSelfContext=true) ))"), *MemberName.ToString(), *ExportMemberParentName);
				}
			}
			else if (MemberGuid.IsValid())
			{
				return FString::Printf(TEXT("Nodes(VariableReference(MemberName=+\"%s\" && MemberGuid(A=%i && B=%i && C=%i && D=%i)))"), *MemberName.ToString(), MemberGuid.A, MemberGuid.B, MemberGuid.C, MemberGuid.D);
			}
			else
			{
				return FString::Printf(TEXT("Nodes(VariableReference(MemberName=+\"%s\"))"), *MemberName.ToString());
			}
		}
		else
		{
			return FString::Printf(TEXT("Nodes(VariableReference(MemberName=+\"%s\" && MemberScope=+\"%s\"))"), *MemberName.ToString(), *GraphProperty.MemberReference.GetMemberScopeName());
		}
	};
	
	//  Native variable.
	const FString SearchString = bUseTempNativeGuid ? GetTempNativeSearchString(Property->GetOwnerClass()) : GetNativeSearchString(Property->GetOwnerClass());
	return GraphProperty.SetGuid(USMUtils::PathToGuid(SearchString), Index, bUseTemplateInGuid);
}

bool FSMNodeInstanceUtils::IsPropertyExposedToGraphNode(const FProperty* Property)
{
	if (!Property)
	{
		return false;
	}

	if (const UScriptStruct* OwnerStruct = Cast<UScriptStruct>(Property->GetOwnerStruct()))
	{
		// Properties that belong to a struct are never displayed on the node.
		return false;
	}
	
	return !Property->HasAnyPropertyFlags(CPF_DisableEditOnInstance) && Property->HasAllPropertyFlags(CPF_BlueprintVisible) &&
		!Property->HasMetaData(TEXT("HideOnNode"));
}

bool FSMNodeInstanceUtils::IsPropertyHandleExposedContainer(const TSharedPtr<IPropertyHandle>& InHandle)
{
	// TODO Containers: If supporting maps or sets this needs to be updated.
	check(InHandle.IsValid());
	return InHandle->AsArray().IsValid() && IsPropertyExposedToGraphNode(InHandle->GetProperty());
}

bool FSMNodeInstanceUtils::ShouldHideNodeStackPropertyFromDetails(const FProperty* InProperty)
{
	const bool bHidden = InProperty->HasMetaData("InstancedTemplate") || InProperty->HasMetaData("NodeBaseOnly");
	return bHidden;
}

bool FSMNodeInstanceUtils::HideEmptyCategoryHandles(const TSharedPtr<IPropertyHandle>& InHandle, ENodeStackType NodeStackType)
{
	if (InHandle.IsValid())
	{
		if (const FProperty* Property = InHandle->GetProperty())
		{
			bool bHidden = false;
			if (NodeStackType != ENodeStackType::None)
			{
				// Stacks should always hide if this property is exposed since a child builder displays that.
				bHidden = ShouldHideNodeStackPropertyFromDetails(Property) ||
					(NodeStackType == ENodeStackType::StateStack && IsPropertyExposedToGraphNode(Property));

				if (!bHidden && NodeStackType == ENodeStackType::TransitionStack)
				{
					const FName PropertyName = Property->GetFName();
					if (PropertyName == TEXT("bUseCustomColors") || PropertyName == TEXT("NodeColor"))
					{
						// State stack allows these to be customized, but they aren't relevant to the transition stack.
						InHandle->MarkHiddenByCustomization();
						bHidden = true;
					}
				}
			}
			else
			{
				// Base states display the properties in their normal categories unless they are containers.
				bHidden = IsPropertyHandleExposedContainer(InHandle);
			}
			return bHidden;
		}

		uint32 HandleNumChildren;
		InHandle->GetNumChildren(HandleNumChildren);

		bool bAreAllChildrenEmpty = true;
		for (uint32 CIdx = 0; CIdx < HandleNumChildren; ++CIdx)
		{
			TSharedPtr<IPropertyHandle> ChildProperty = InHandle->GetChildHandle(CIdx);
			const bool bIsChildEmpty = HideEmptyCategoryHandles(ChildProperty, NodeStackType);
			if (!bIsChildEmpty)
			{
				bAreAllChildrenEmpty = false;
				continue;
			}

			ChildProperty->MarkHiddenByCustomization();
		}

		if (bAreAllChildrenEmpty)
		{
			InHandle->MarkHiddenByCustomization();
		}

		return bAreAllChildrenEmpty;
	}

	return true;
}

FStructProperty* FSMNodeInstanceUtils::GetGraphPropertyFromProperty(FProperty* Property)
{
	if (Property->HasMetaData(TEXT("HideOnNode")))
	{
		// Assume this node never wants to be displayed.
		return nullptr;
	}
	
	if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
	{
		if (StructProperty->Struct->IsChildOf(FSMGraphProperty_Base_Runtime::StaticStruct()))
		{
			return StructProperty;
		}
	}

	if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		if (FStructProperty* StructProperty = CastField<FStructProperty>(ArrayProperty->Inner))
		{
			if (StructProperty->Struct->IsChildOf(FSMGraphProperty_Base_Runtime::StaticStruct()))
			{
				return StructProperty;
			}
		}
	}

	return nullptr;
}

bool FSMNodeInstanceUtils::IsPropertyGraphProperty(const FProperty* Property)
{
	return GetGraphPropertyFromProperty(const_cast<FProperty*>(Property)) != nullptr;
}

bool FSMNodeInstanceUtils::DoesNodeClassPossiblyHaveConstructionScripts(TSubclassOf<USMNodeInstance> NodeClass, ESMExecutionEnvironment ExecutionType)
{
	if (!NodeClass)
	{
		return false;
	}

	const bool bIsBaseClass = FSMNodeClassRule::IsBaseClass(NodeClass);
	if (bIsBaseClass)
	{
		// Base classes have no construction script logic.
		return false;
	}

	auto DoesGraphHaveUserLogic = [ExecutionType](UEdGraph* InGraph, bool& bOutHasParentCall) -> bool
	{
		UK2Node_FunctionEntry* EntryNode = FSMBlueprintEditorUtils::GetFirstNodeOfClassNested<UK2Node_FunctionEntry>(InGraph);
		if (!ensure(EntryNode))
		{
			return false;
		}

		UEdGraphPin* ThenPin = EntryNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);
		if (ThenPin->LinkedTo.Num() == 0)
		{
			// No connections, no logic.
			return false;
		}

		UK2Node_CallFunction* ExecutionEnvironmentFunction = nullptr;
		
		UK2Node_CallParentFunction* ParentCall = Cast<UK2Node_CallParentFunction>(ThenPin->LinkedTo[0]->GetOwningNode());
		if (ParentCall == nullptr)
		{
			// Check if instead of the parent we are connected right to the with execution node.
			ExecutionEnvironmentFunction = Cast<UK2Node_CallFunction>(ThenPin->LinkedTo[0]->GetOwningNode());
			if (ExecutionEnvironmentFunction == nullptr)
			{
				// This isn't a default layout, assume user logic.
				return true;
			}
		}
		else
		{
			bOutHasParentCall = true;
		}
		
		if (ParentCall && ParentCall->GetThenPin()->LinkedTo.Num() == 0)
		{
			// No connections, no logic.
			return false;
		}

		if (ExecutionEnvironmentFunction == nullptr && ParentCall != nullptr)
		{
			ExecutionEnvironmentFunction = Cast<UK2Node_CallFunction>(ParentCall->GetThenPin()->LinkedTo[0]->GetOwningNode());
		}
		
		if (!ExecutionEnvironmentFunction || ExecutionEnvironmentFunction->GetFunctionName() != GET_FUNCTION_NAME_CHECKED(USMNodeInstance, WithExecutionEnvironment))
		{
			// Unexpected type or different function, assume user logic.
			return true;
		}

		if (ExecutionType == ESMExecutionEnvironment::EditorExecution)
		{
			if (UEdGraphPin* EditorExecutionPin = ExecutionEnvironmentFunction->FindPin(TEXT("EditorExecution"), EGPD_Output))
			{
				if (EditorExecutionPin->LinkedTo.Num() > 0)
				{
					// Editor output pin is connected somewhere, there is user logic.
					return true;
				}
			}
		}
		else if (ExecutionType == ESMExecutionEnvironment::GameExecution)
		{
			if (UEdGraphPin* GameExecutionPin = ExecutionEnvironmentFunction->FindPin(TEXT("GameExecution"), EGPD_Output))
			{
				if (GameExecutionPin->LinkedTo.Num() > 0)
				{
					// Game output pin is connected somewhere, there is user logic.
					return true;
				}
			}
		}
		
		return false;
	};
	
	if (USMNodeBlueprint* NodeBlueprint = Cast<USMNodeBlueprint>(UBlueprint::GetBlueprintFromClass(NodeClass)))
	{
		if (TObjectPtr<UEdGraph>* ConstructionScriptGraph = NodeBlueprint->FunctionGraphs.FindByPredicate([] (UEdGraph* InGraph)
		{
			return InGraph->GetFName() == USMNodeInstance::GetConstructionScriptFunctionName();
		}))
		{
			bool bHasParentCall = false;
			const bool bHasGraphLogic = DoesGraphHaveUserLogic(*ConstructionScriptGraph, bHasParentCall);
			if (bHasGraphLogic)
			{
				return true;
			}

			// No graph logic, check parents.
			if (bHasParentCall)
			{
				return DoesNodeClassPossiblyHaveConstructionScripts(NodeClass->GetSuperClass(), ExecutionType);
			}

			return false;
		}
	}

	// No blueprint or graph found... probably a native class.
	if (USMNodeInstance* NodeDefaults = Cast<USMNodeInstance>(NodeClass->GetDefaultObject()))
	{
		return !NodeDefaults->ShouldSkipNativeEditorConstructionScripts();
	}

	return true;
}

TSharedPtr<IPropertyHandle> FSMNodeInstanceUtils::FindExposedPropertyOverrideByName(USMNodeInstance* InNodeInstance,
	const FName& VariableName, TSharedPtr<ISinglePropertyView>& OutPropView)
{
	check(InNodeInstance);
	
	OutPropView = LD::PropertyUtils::CreatePropertyViewForProperty(InNodeInstance, GET_MEMBER_NAME_CHECKED(USMNodeInstance, ExposedPropertyOverrides));

	const TSharedPtr<IPropertyHandle> PropertyHandle = OutPropView->GetPropertyHandle();
	check(PropertyHandle->IsValidHandle());

	const TSharedPtr<IPropertyHandleArray> ArrayPropertyHandle = PropertyHandle->AsArray();
	check(ArrayPropertyHandle.IsValid());
	
	uint32 NumElements = 0;
	{
		const FPropertyAccess::Result Result = ArrayPropertyHandle->GetNumElements(NumElements);
		check(Result == FPropertyAccess::Success);
	}

	for (uint32 Idx = 0; Idx < NumElements; ++Idx)
	{
		const TSharedRef<IPropertyHandle> Element = ArrayPropertyHandle->GetElement(Idx);
		check(Element->IsValidHandle());

		void* Data;
		const FPropertyAccess::Result Result = Element->GetValueData(Data);
		check(Result == FPropertyAccess::Success);

		const FSMGraphProperty* GraphProperty = static_cast<FSMGraphProperty*>(Data);
		if (GraphProperty->VariableName == VariableName)
		{
			return Element;
		}
	}

	return nullptr;
}

TSharedPtr<IPropertyHandle> FSMNodeInstanceUtils::FindOrAddExposedPropertyOverrideByName(USMNodeInstance* InNodeInstance,
                                                                                         const FName& VariableName, TSharedPtr<ISinglePropertyView>& OutPropView)
{
	check(InNodeInstance);

	// First look for an existing element.
	TSharedPtr<IPropertyHandle> ExistingPropertyHandle = FindExposedPropertyOverrideByName(InNodeInstance, VariableName, OutPropView);
	if (ExistingPropertyHandle.IsValid())
	{
		return ExistingPropertyHandle;
	}
	
	const bool bPackageWasDirty = InNodeInstance->GetPackage()->IsDirty();

	const TSharedPtr<IPropertyHandle> PropertyHandle = OutPropView->GetPropertyHandle();
	check(PropertyHandle->IsValidHandle());

	const TSharedPtr<IPropertyHandleArray> ArrayPropertyHandle = PropertyHandle->AsArray();
	check(ArrayPropertyHandle.IsValid());
	
	// Not found, add a new one.
	{
		const FPropertyAccess::Result Result = ArrayPropertyHandle->AddItem();
		check(Result == FPropertyAccess::Success);
	}

	uint32 NumElements = 0;
	{
		const FPropertyAccess::Result Result = ArrayPropertyHandle->GetNumElements(NumElements);
		check(Result == FPropertyAccess::Success);
	}

	TSharedPtr<IPropertyHandle> ElementHandle = ArrayPropertyHandle->GetElement(NumElements - 1);
	check(ElementHandle->IsValidHandle());
	
	// Set the correct name.
	{
		const TSharedPtr<IPropertyHandle> NameHandle = ElementHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSMGraphProperty, VariableName));
		check(NameHandle->IsValidHandle());
	
		NameHandle->SetValue(VariableName);
	}

	if (!bPackageWasDirty)
	{
		// It's okay not to save this on initial add. It only needs to be saved if a value has changed. Otherwise just
		// clicking on a variable could dirty the asset.
		InNodeInstance->GetPackage()->ClearDirtyFlag();
	}

	return ElementHandle;
}

bool FSMNodeInstanceUtils::UpdateExposedPropertyOverrideName(USMNodeInstance* InNodeInstance, const FName& OldVarName,
	const FName& NewVarName)
{
	TSharedPtr<ISinglePropertyView> PropView;
	const TSharedPtr<IPropertyHandle> ExistingPropertyHandle = FindExposedPropertyOverrideByName(InNodeInstance, OldVarName, PropView);
	if (ExistingPropertyHandle.IsValid())
	{
		const TSharedPtr<IPropertyHandle> NameHandle = ExistingPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSMGraphProperty, VariableName));
		check(NameHandle.IsValid());
		
		NameHandle->SetValue(NewVarName);
		return true;
	}

	return false;
}

uint32 FSMNodeInstanceUtils::RemoveExposedPropertyOverrideByName(USMNodeInstance* InNodeInstance, const FName& VariableName)
{
	const TSharedPtr<ISinglePropertyView> PropertyView =
		LD::PropertyUtils::CreatePropertyViewForProperty(InNodeInstance, GET_MEMBER_NAME_CHECKED(USMNodeInstance, ExposedPropertyOverrides));

	const TSharedPtr<IPropertyHandle> PropertyHandle = PropertyView->GetPropertyHandle();
	check(PropertyHandle->IsValidHandle());

	const TSharedPtr<IPropertyHandleArray> ArrayPropertyHandle = PropertyHandle->AsArray();
	check(ArrayPropertyHandle.IsValid());
	
	uint32 NumElements = 0;
	{
		const FPropertyAccess::Result Result = ArrayPropertyHandle->GetNumElements(NumElements);
		check(Result == FPropertyAccess::Success);
	}

	uint32 ElementsRemoved = 0;
	for (uint32 Idx = 0; Idx < NumElements;)
	{
		const TSharedRef<IPropertyHandle> Element = ArrayPropertyHandle->GetElement(Idx);
		check(Element->IsValidHandle());

		void* Data;
		{
			const FPropertyAccess::Result Result = Element->GetValueData(Data);
			check(Result == FPropertyAccess::Success);
		}
		
		const FSMGraphProperty* GraphProperty = static_cast<FSMGraphProperty*>(Data);
		if (GraphProperty->VariableName == VariableName)
		{
			const FPropertyAccess::Result Result = ArrayPropertyHandle->DeleteItem(Idx);
			check(Result == FPropertyAccess::Success);
			
			++ElementsRemoved;
			--NumElements;
		}
		else
		{
			++Idx;
		}
	}

	return ElementsRemoved;
}

#undef LOCTEXT_NAMESPACE
