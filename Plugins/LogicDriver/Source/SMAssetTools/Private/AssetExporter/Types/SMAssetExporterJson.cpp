// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "AssetExporter/Types/SMAssetExporterJson.h"

#include "Utilities/SMImportExportUtils.h"
#include "Utilities/SMJsonUtils.h"

#include "SMInstance.h"
#include "Blueprints/SMBlueprintGeneratedClass.h"

#include "Graph/SMConduitGraph.h"
#include "Graph/SMGraph.h"
#include "Graph/SMTransitionGraph.h"
#include "Graph/Nodes/SMGraphNode_Base.h"
#include "Graph/Nodes/SMGraphNode_ConduitNode.h"
#include "Graph/Nodes/SMGraphNode_StateMachineEntryNode.h"
#include "Graph/Nodes/SMGraphNode_StateNode.h"
#include "Graph/Nodes/SMGraphNode_TransitionEdge.h"
#include "Utilities/SMBlueprintEditorUtils.h"

#include "JsonObjectConverter.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"

#define NODE_BASE_REMOVAL TEXT("%NODE_BASE_REMOVAL_e19a77a2-19cc-449f-a79a-e5267bb9d1f4%")

USMAssetExporter::EExportStatus USMAssetExporterJson::OnBeginExport(const FExportArgs& InExportArgs)
{
	check(InExportArgs.Blueprint.IsValid());

	ExportArgs = InExportArgs;
	RootJsonObject = MakeShared<FJsonObject>();

	if (!ensure(InExportArgs.Blueprint->GetGeneratedClass()))
	{
		return EExportStatus::Failure;
	}

	const TSoftClassPtr<USMInstance> ParentClass = InExportArgs.Blueprint->GetGeneratedClass()->GetSuperClass();
	check(ParentClass.IsValid());

	const FSMNode_Base* RuntimeNode = FSMBlueprintEditorUtils::GetRuntimeNodeFromGraph(FSMBlueprintEditorUtils::GetRootStateMachineGraph(InExportArgs.Blueprint.Get()));
	check(RuntimeNode);

	RootJsonObject->SetNumberField(LD::JsonFields::FIELD_JSON_VERSION, LD::JsonUtils::CurrentVersion);
	RootJsonObject->SetStringField(LD::JsonFields::FIELD_NAME, InExportArgs.Blueprint->GetName());
	RootJsonObject->SetStringField(LD::JsonFields::FIELD_PARENT_CLASS, ParentClass.ToString());
	RootJsonObject->SetStringField(LD::JsonFields::FIELD_ROOT_GUID, RuntimeNode->GetNodeGuid().ToString());

	return EExportStatus::Success;
}

USMAssetExporter::EExportStatus USMAssetExporterJson::OnExportCDO(const UObject* InCDO)
{
	const TSharedPtr<FJsonObject> JsonCDO = CreateJsonObject(InCDO);
	RootJsonObject->SetObjectField(LD::JsonFields::FIELD_CDO, JsonCDO);

	return EExportStatus::Success;
}

USMAssetExporter::EExportStatus USMAssetExporterJson::OnExportNode(const USMGraphNode_Base* InGraphNode)
{
	TSharedPtr<FJsonValueObject> NodeJsonValue;
	const EExportStatus Status = GraphNodeToJsonValue(InGraphNode, NodeJsonValue);

	if (Status == EExportStatus::Success && NodeJsonValue.IsValid())
	{
		if (const USMGraphNode_StateNodeBase* StateNode = Cast<USMGraphNode_StateNodeBase>(InGraphNode))
		{
			StateJsonArray.Add(NodeJsonValue);
		}
		else if (const USMGraphNode_TransitionEdge* Transition = Cast<USMGraphNode_TransitionEdge>(InGraphNode))
		{
			TransitionJsonArray.Add(NodeJsonValue);
		}
		else if (const USMGraphNode_StateMachineEntryNode* EntryNode = Cast<USMGraphNode_StateMachineEntryNode>(InGraphNode))
		{
			EntryJsonArray.Add(NodeJsonValue);
		}
	}

	return Status;
}

void USMAssetExporterJson::OnFinishExport(USMBlueprint* InBlueprint, EExportStatus InStatus)
{
	switch(InStatus)
	{
	case EExportStatus::Failure:
		return;
	case EExportStatus::Success:
		break;
	}

	// Create state map of {NodeGuid -> NodeObject}
	{
		const TSharedPtr<FJsonObject> StateJsonMap = MakeShared<FJsonObject>();
		for (const TSharedPtr<FJsonValue>& StateJsonValue : StateJsonArray)
		{
			const TSharedPtr<FJsonObject>& StateJsonObject = StateJsonValue->AsObject();
			FString JsonGuid = StateJsonObject->GetStringField(LD::JsonFields::FIELD_NODE_GUID);
			StateJsonMap->SetObjectField(JsonGuid, StateJsonObject);
		}
		RootJsonObject->SetObjectField(LD::JsonFields::FIELD_STATES, StateJsonMap);
	}

	// Create transition map of {NodeGuid -> NodeObject}
	{
		const TSharedPtr<FJsonObject> TransitionJsonMap = MakeShared<FJsonObject>();
		for (const TSharedPtr<FJsonValue>& TransitionJsonValue : TransitionJsonArray)
		{
			const TSharedPtr<FJsonObject>& TransitionJsonObject = TransitionJsonValue->AsObject();
			FString JsonGuid = TransitionJsonObject->GetStringField(LD::JsonFields::FIELD_NODE_GUID);
			TransitionJsonMap->SetObjectField(JsonGuid, TransitionJsonObject);
		}
		RootJsonObject->SetObjectField(LD::JsonFields::FIELD_TRANSITIONS, TransitionJsonMap);
	}

	RootJsonObject->SetArrayField(LD::JsonFields::FIELD_ENTRY_NODES, EntryJsonArray);

	// Cleanup after object is complete.
	CleanupJsonObject(RootJsonObject);

	if (ExportArgs.bMemoryOnly)
	{
		return;
	}

	if (ensure(!ExportArgs.ExportFullFilePath.IsEmpty()))
	{
		if (FArchive* FileWriter = IFileManager::Get().CreateFileWriter(*ExportArgs.ExportFullFilePath))
		{
			const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(FileWriter, 0);
			FJsonSerializer::Serialize(RootJsonObject.ToSharedRef(), JsonWriter);

			ensure(FileWriter->Close());
			delete FileWriter;
		}
	}
}

USMAssetExporter::EExportStatus USMAssetExporterJson::GraphNodeToJsonValue(const USMGraphNode_Base* InGraphNode,
	TSharedPtr<FJsonValueObject>& OutJsonValue)
{
	if (InGraphNode && InGraphNode->GetClass()->HasMetaData(TEXT("NoLogicDriverExport")))
	{
		return EExportStatus::Success;
	}

	const TSharedPtr<FJsonValueObject> NodeJsonValue = CreateJsonValueObject(InGraphNode);
	if (!NodeJsonValue.IsValid())
	{
		return EExportStatus::Failure;
	}

	const USMGraph* OwningSMGraph = InGraphNode->GetOwningStateMachineGraph();
	check(OwningSMGraph);
	const USMGraphNode_StateMachineEntryNode* InGraphEntryNode = OwningSMGraph->GetEntryNode();
	check(InGraphEntryNode);
	const FGuid OwningNodeGuid = InGraphEntryNode->StateMachineNode.GetNodeGuid();

	const TSharedPtr<FJsonObject>& JsonObject = NodeJsonValue->AsObject();
	JsonObject->SetStringField(LD::JsonFields::FIELD_GRAPH_NODE_CLASS, InGraphNode->GetClass()->GetPathName());
	JsonObject->SetStringField(LD::JsonFields::FIELD_OWNER_GUID, OwningNodeGuid.ToString());
	JsonObject->SetStringField(LD::JsonFields::FIELD_NAME, InGraphNode->GetNodeName());

	const FGuid& Guid = InGraphNode->GetCorrectNodeGuid();
	if (ensure(Guid.IsValid()))
	{
		JsonObject->SetStringField(LD::JsonFields::FIELD_NODE_GUID, Guid.ToString());
	}

	if (const USMGraphNode_ConduitNode* ConduitNode = Cast<USMGraphNode_ConduitNode>(InGraphNode))
	{
		if (CastChecked<USMConduitGraph>(ConduitNode->GetBoundGraph())->GetConditionalEvaluationType() == ESMConditionalEvaluationType::SM_AlwaysTrue)
		{
			JsonObject->SetBoolField(LD::JsonFields::FIELD_EVAL_DEFAULT, true);
		}
	}

	if (const USMGraphNode_StateNodeBase* StateNode = Cast<USMGraphNode_StateNodeBase>(InGraphNode))
	{
		// Determine if this is an entry state.
		TArray<USMGraphNode_StateNodeBase*> InitialStateNodes;
		InGraphEntryNode->GetAllOutputNodesAs(InitialStateNodes);
		JsonObject->SetBoolField(LD::JsonFields::FIELD_CONNECTED_TO_ENTRY, InitialStateNodes.Contains(StateNode));
	}
	else if (const USMGraphNode_TransitionEdge* Transition = Cast<USMGraphNode_TransitionEdge>(InGraphNode))
	{
		const FGuid& FromGuid = Transition->GetFromState()->GetCorrectNodeGuid();
		check(FromGuid.IsValid());
		JsonObject->SetStringField(LD::JsonFields::FIELD_FROM_GUID, FromGuid.ToString());

		const FGuid& ToGuid = Transition->GetToState()->GetCorrectNodeGuid();
		check(ToGuid.IsValid());
		JsonObject->SetStringField(LD::JsonFields::FIELD_TO_GUID, ToGuid.ToString());

		if (Transition->GetTransitionGraph()->GetConditionalEvaluationType() == ESMConditionalEvaluationType::SM_AlwaysTrue)
		{
			JsonObject->SetBoolField(LD::JsonFields::FIELD_EVAL_DEFAULT, true);
		}
	}

	OutJsonValue = NodeJsonValue;

	return EExportStatus::Success;
}

TSharedPtr<FJsonObject> USMAssetExporterJson::CreateJsonObject(const UObject* InObject)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	if (InObject)
	{
		for (TFieldIterator<FProperty> PropertyItr(InObject->GetClass()); PropertyItr; ++PropertyItr)
		{
			FProperty* Property = *PropertyItr;
			if (LD::ImportExportUtils::ShouldPropertyBeImportedOrExported(Property))
			{
				// Export properties that meet our criteria. Unfortunately metadata can't be checked for skipping,
				// so we have to setup a callback to do a full property check later.

				const void* Value = Property->ContainerPtrToValuePtr<void>(InObject);

				if (Property->HasAnyPropertyFlags(CPF_ExportObject) && !Property->HasAnyPropertyFlags(CPF_InstancedReference))
				{
					// Exported and non-instanced objects are handled differently by the Json serializer. By default
					// it only saves the object path, but we need to save all properties.
					if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
					{
						if (const UObject* ObjectValue = ObjectProperty->GetObjectPropertyValue(Value))
						{
							TSharedPtr<FJsonObject> ExportedObject = CreateJsonObject(ObjectValue);
							// Needed so the json deserializer can restore on import.
							ExportedObject->SetStringField(TEXT("_ClassName"), ObjectValue->GetClass()->GetFName().ToString());
							JsonObject->SetObjectField(Property->GetName(), ExportedObject);
							continue;
						}
					}
				}

				const uint64 CheckFlags = CPF_BlueprintVisible | CPF_Edit | CPF_ContainsInstancedReference;
				const uint64 SkipFlags = CPF_Transient;
				auto Delegate = FJsonObjectConverter::CustomExportCallback::CreateStatic(&USMAssetExporterJson::OnExportJsonProperty);
				TSharedPtr<FJsonValue> JsonValue = FJsonObjectConverter::UPropertyToJsonValue(Property, Value,
					CheckFlags, SkipFlags, &Delegate);
				if (JsonValue.IsValid())
				{
					JsonObject->SetField(Property->GetName(), MoveTemp(JsonValue));
				}
			}
		}
	}

	return MoveTemp(JsonObject);
}

TSharedPtr<FJsonValueObject> USMAssetExporterJson::CreateJsonValueObject(const UObject* InObject)
{
	TSharedPtr<FJsonObject> JsonObject = CreateJsonObject(InObject);
	if (JsonObject.IsValid())
	{
		return MakeShareable(new FJsonValueObject(MoveTemp(JsonObject)));
	}

	return nullptr;
}

void USMAssetExporterJson::CleanupJsonObject(TSharedPtr<FJsonObject> JsonObject)
{
	for (auto ObjIt = JsonObject->Values.CreateIterator(); ObjIt; ++ObjIt)
	{
		// Check strings.
		{
			FString ValueString;
			if (ObjIt->Value->TryGetString(ValueString))
			{
				if (ValueString == NODE_BASE_REMOVAL)
				{
					ObjIt.RemoveCurrent();
				}
				continue;
			}
		}

		// Check objects.
		{
			const TSharedPtr<FJsonObject>* ObjectValue = nullptr;
			if (ObjIt->Value->TryGetObject(ObjectValue))
			{
				CleanupJsonObject(*ObjectValue);
			}
		}

		// Check arrays.
		{
			const TArray<TSharedPtr<FJsonValue>>* ArrayValue = nullptr;
			if (ObjIt->Value->TryGetArray(ArrayValue))
			{
				// Const cast necessary for removal.
				for (auto ArrayIt = const_cast<TArray<TSharedPtr<FJsonValue>>*>(ArrayValue)->CreateIterator(); ArrayIt; ++ArrayIt)
				{
					FString ValueString;
					if ((*ArrayIt)->TryGetString(ValueString))
					{
						if (ValueString == NODE_BASE_REMOVAL)
						{
							ArrayIt.RemoveCurrent();
						}

						continue;
					}

					const TSharedPtr<FJsonObject>* ObjectValue = nullptr;
					if ((*ArrayIt)->TryGetObject(ObjectValue))
					{
						CleanupJsonObject(*ObjectValue);
					}
				}
			}
		}
	}
}

TSharedPtr<FJsonValue> USMAssetExporterJson::OnExportJsonProperty(FProperty* InProperty, const void* Value)
{
	if (LD::ImportExportUtils::ShouldPropertyBeImportedOrExported(InProperty))
	{
		// Valid property, UE will handle.
		return nullptr;
	}

	// Signal for later that this property should be removed. We can't skip the property here and this is kind of hacky,
	// but is a fairly simple solution to allow UE to handle nested structs and object instances for us.
	TSharedPtr<FJsonValueString> EmptyResult = MakeShared<FJsonValueString>(NODE_BASE_REMOVAL);
	return EmptyResult;
}
