// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#include "SMPropertyUtils.h"

#include "Utilities/SMBlueprintEditorUtils.h"
#include "Utilities/SMNodeInstanceUtils.h"

#include "ISinglePropertyView.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"

TSharedPtr<ISinglePropertyView> LD::PropertyUtils::CreatePropertyViewForProperty(UObject* InObjectOwner,
                                                                               const FName& InPropertyName)
{
	check(InObjectOwner);
	
	const FSinglePropertyParams InitParams;
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedPtr<ISinglePropertyView> PropertyView = PropertyEditorModule.CreateSingleProperty(
		InObjectOwner,
		InPropertyName,
		InitParams);

	check(PropertyView.IsValid());
	return PropertyView;
}

/** Sets a single property value with no array handling. */
void SetSinglePropertyValueImpl(const FProperty* InProperty, const FString& InValue, uint8* Container, UObject* InObject)
{
	if (!FBlueprintEditorUtils::PropertyValueFromString_Direct(InProperty, InValue, Container, InObject))
	{
		// Fallback to generic import, don't log as this is common when adding variables and not changing their defaults.
		InProperty->ImportText_Direct(*InValue, Container, InObject, PPF_SerializedAsImportText);
	}
}

void LD::PropertyUtils::SetPropertyValue(FProperty* InProperty, const FString& InValue, UObject* InObject, int32 InArrayIndex)
{
	if (InObject)
	{
		InObject->Modify();
	}

	// The final property that will have its value set.
	FProperty* PropertyToSet = InProperty;

	// The owner of the container which could be the object or the container if an array.
	uint8* ContainerOwner = reinterpret_cast<uint8*>(InObject);

	// The immediate container of the property.
	uint8* Container = nullptr;

	if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(InProperty))
	{
		FScriptArrayHelper Helper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<uint8>(InObject));
		if (Helper.IsValidIndex(InArrayIndex))
		{
			PropertyToSet = ArrayProperty->Inner;
			Container = Helper.GetRawPtr(InArrayIndex);
			ContainerOwner = Container;
		}
	}
	else
	{
		Container = InProperty->ContainerPtrToValuePtr<uint8>(ContainerOwner, InArrayIndex);
	}

	// Check for extended graph property.
	if (FProperty* ExtendGraphResultProperty = GetExtendedGraphPropertyResult(PropertyToSet, ContainerOwner))
	{
		Container = ExtendGraphResultProperty->ContainerPtrToValuePtr<uint8>(Container);
		PropertyToSet = ExtendGraphResultProperty;
	}

	if (ensure(Container))
	{
		SetSinglePropertyValueImpl(PropertyToSet, InValue, Container, InObject);
	}
}

FString LD::PropertyUtils::GetPropertyValue(FProperty* InProperty, UObject* InObject, int32 InArrayIndex)
{
	// The final property that will have its value returned.
	FProperty* PropertyToGet = InProperty;

	// The owner of the container which could be the object or the container if an array.
	uint8* ContainerOwner = reinterpret_cast<uint8*>(InObject);

	// The immediate container of the property.
	uint8* Container = nullptr;

	if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(InProperty))
	{
		FScriptArrayHelper Helper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<uint8>(InObject));
		if (!(ensureMsgf(Helper.IsValidIndex(InArrayIndex), TEXT("Invalid array index given to GetPropertyValue for array %s."), *InProperty->GetName())))
		{
			return FString();
		}

		PropertyToGet = ArrayProperty->Inner;
		Container = Helper.GetRawPtr(InArrayIndex);
		ContainerOwner = Container;
	}
	else
	{
		Container = InProperty->ContainerPtrToValuePtr<uint8>(ContainerOwner, InArrayIndex);
	}

	// Check for extended graph property.
	if (FProperty* ExtendGraphResultProperty = GetExtendedGraphPropertyResult(PropertyToGet, ContainerOwner))
	{
		Container = ExtendGraphResultProperty->ContainerPtrToValuePtr<uint8>(Container);
		PropertyToGet = ExtendGraphResultProperty;
	}

	FString Result;
	if (ensure(Container))
	{
		FBlueprintEditorUtils::PropertyValueToString_Direct(PropertyToGet, Container, Result, InObject);
	}

	return MoveTemp(Result);
}

FProperty* LD::PropertyUtils::GetExtendedGraphPropertyResult(FProperty* InProperty, uint8* InContainer)
{
	if (const FStructProperty* StructProperty = FSMNodeInstanceUtils::GetGraphPropertyFromProperty(InProperty))
	{
		check(InContainer);

		// Access the graph property instance so can find the virtual result property name.
		const FSMGraphProperty_Base* GraphProperty = InProperty->ContainerPtrToValuePtr<FSMGraphProperty_Base>(InContainer);
		check(GraphProperty);

		const FName ResultPropertyName = GraphProperty->GetResultPropertyName();
		if (!ResultPropertyName.IsNone())
		{
			// A result property indicates this is a custom graph property that has a sub property managing the value.
			FProperty* ResultProperty = StructProperty->Struct->FindPropertyByName(ResultPropertyName);
			check(ResultProperty);

			return ResultProperty;
		}
	}

	return nullptr;
}

bool LD::PropertyUtils::IsObjectPropertyInstanced(const FObjectProperty* ObjectProperty)
{
	return ObjectProperty && ObjectProperty->HasAnyPropertyFlags(CPF_InstancedReference | CPF_ExportObject);
}

UObject* LD::PropertyUtils::FPropertyRetrieval::GetObjectValue() const
{
	if (ObjectProperty && ObjectContainer)
	{
		return ObjectProperty->GetObjectPropertyValue(ObjectContainer);
	}
	return nullptr;
}

void LD::PropertyUtils::FPropertyRetrieval::SetObjectValue(UObject* NewValue) const
{
	if (ObjectProperty && ObjectContainer)
	{
		return ObjectProperty->SetObjectPropertyValue(const_cast<void*>(ObjectContainer), NewValue);
	}
}

namespace LD
{
	namespace PropertyUtils
	{
		void GetAllObjectPropertiesImpl(const void* InObject, const UStruct* InPropertySource,
													   TArray<FPropertyRetrieval>& OutProperties,
													   TSet<const void*>* ObjectsChecked, const FPropertyRetrievalArgs& InArgs)
		{
			if (!InObject || ObjectsChecked->Contains(InObject))
			{
				return;
			}

			ObjectsChecked->Add(InObject);

			auto ProcessProperty = [&] (FProperty* Property, const void* Object)
			{
				if (!Object)
				{
					return;
				}

				ObjectsChecked->Add(Object);

				if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
				{
					const void* StructAddressInObject = StructProperty->ContainerPtrToValuePtr<const void>(Object);
					GetAllObjectPropertiesImpl(StructAddressInObject, StructProperty->Struct, OutProperties, ObjectsChecked, InArgs);
				}
				else if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
				{
					// Check for any filters.
					if (InArgs.IncludePropertyFlags != CPF_None && !Property->HasAllPropertyFlags(InArgs.IncludePropertyFlags))
					{
						return;
					}
					if (InArgs.ExcludePropertyFlags != CPF_None && Property->HasAnyPropertyFlags(InArgs.ExcludePropertyFlags))
					{
						return;
					}

					const void* ObjectContainer = ObjectProperty->ContainerPtrToValuePtr<const void>(Object);
					const FPropertyRetrieval PropertyRetrieval{ ObjectProperty, ObjectContainer };
					OutProperties.Add(PropertyRetrieval);

					if (IsObjectPropertyInstanced(ObjectProperty))
					{
						// Only check property instances stored within this object.
						if (const UObject* ObjectValue = PropertyRetrieval.GetObjectValue())
						{
							GetAllObjectPropertiesImpl(ObjectValue, ObjectValue->GetClass(), OutProperties, ObjectsChecked, InArgs);
						}
					}
				}
			};

			for (TFieldIterator<FProperty> PropertyIterator(InPropertySource); PropertyIterator; ++PropertyIterator)
			{
				FProperty* Property = *PropertyIterator;

				if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
				{
					FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(InObject));
					for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
					{
						const uint8* ArrayData = ArrayHelper.GetRawPtr(Index);
						ProcessProperty(ArrayProperty->Inner, ArrayData);
					}
				}
				/*
				else if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
				{
					// TODO: MapProperty
				}
				else if (const FSetProperty* SetProperty = CastField<FSetProperty>(Property))
				{
					// TODO: SetProperty
				}*/
				else
				{
					ProcessProperty(Property, InObject);
				}
			}
		}
	}
}

void LD::PropertyUtils::GetAllObjectProperties(const void* InObject, const UStruct* InPropertySource,
	TArray<FPropertyRetrieval>& OutProperties, const FPropertyRetrievalArgs& InArgs)
{
	TSet<const void*> ObjectsCheckedSource;
	GetAllObjectPropertiesImpl(InObject, InPropertySource, OutProperties, &ObjectsCheckedSource, InArgs);
}

void LD::PropertyUtils::ForEachInstancedSubObject(const UObject* InObject, const TFunction<void(UObject*)> Function)
{
	// Verify there are sub objects first before finding properties. This saves a call to GetAllObjectProperties which
	// is much slower.
	bool bHasSubObjects = false;
	ForEachObjectWithOuter(InObject, [&](const UObject* Child)
	{
		if (IsValid(Child))
		{
			bHasSubObjects = true;
		}
	});

	if (bHasSubObjects)
	{
		TArray<FPropertyRetrieval> ObjectProperties;
		FPropertyRetrievalArgs Args;
		Args.IncludePropertyFlags = CPF_InstancedReference;
		Args.ExcludePropertyFlags = CPF_Transient;
		GetAllObjectProperties(InObject, InObject->GetClass(),
			ObjectProperties, MoveTemp(Args));
		for (FPropertyRetrieval& PropertyRetrieval : ObjectProperties)
		{
			if (!PropertyRetrieval.ObjectProperty ||
				!ensure(!PropertyRetrieval.ObjectProperty->HasAnyPropertyFlags(CPF_Transient)))
			{
				continue;
			}

			if (UObject* ObjectValue = PropertyRetrieval.GetObjectValue())
			{
				Function(ObjectValue);
			}
		}
	}
}
