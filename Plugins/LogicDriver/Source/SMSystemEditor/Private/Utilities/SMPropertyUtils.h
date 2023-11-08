// Copyright Recursoft LLC 2019-2022. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

class ISinglePropertyView;

namespace LD
{
	namespace PropertyUtils
	{
		/**
		* Create a temporary property handle for a given property. Call GetPropertyHandle() from here.
		* The property handle will be valid as long as the property view is valid.
		*
		* @param InObjectOwner UObject owning the property.
		* @param InPropertyName The field name of the property.
		*
		* @return A property handle created for this property.
		*/
		SMSYSTEMEDITOR_API TSharedPtr<ISinglePropertyView> CreatePropertyViewForProperty(UObject* InObjectOwner, const FName& InPropertyName);

		/**
		 * Correctly set a property value by importing a text value. Handles individual properties, arrays, and extended graph properties.
		 * This does not use property handles and instance propagation will not occur.
		 *
		 * @param InProperty The property to import data.
		 * @param InValue The text value to import.
		 * @param InObject The object instance containing this property.
		 * @param InArrayIndex An index for the property.
		 */
		SMSYSTEMEDITOR_API void SetPropertyValue(FProperty* InProperty, const FString& InValue, UObject* InObject, int32 InArrayIndex = 0);

		/**
		 * Return the string value of a property. Handles individual properties, arrays, and extended graph properties.
		 *
		 * @param InProperty The property to import data.
		 * @param InObject The object instance containing this property.
		 * @param InArrayIndex An index for the property.
		 */
		SMSYSTEMEDITOR_API FString GetPropertyValue(FProperty* InProperty, UObject* InObject, int32 InArrayIndex = 0);

		/**
		 * Retrieve the 'Result' FProperty from a single property with no array handling. Null if not a custom graph property.
		 *
		 * @param InProperty The extended graph property.
		 * @param InContainer The instance container of the property.
		 *
		 * @return The FProperty result, or nullptr if not a custom graph property.
		 */
		SMSYSTEMEDITOR_API FProperty* GetExtendedGraphPropertyResult(FProperty* InProperty, uint8* InContainer);

		/** Checks if an object property is instanced or exported. */
		SMSYSTEMEDITOR_API bool IsObjectPropertyInstanced(const FObjectProperty* ObjectProperty);

		struct SMSYSTEMEDITOR_API FPropertyRetrieval
		{
			FObjectProperty* ObjectProperty;
			const void* ObjectContainer;

			UObject* GetObjectValue() const;
			void SetObjectValue(UObject* NewValue) const;
		};

		struct FPropertyRetrievalArgs
		{
			FPropertyRetrievalArgs() : IncludePropertyFlags(CPF_None), ExcludePropertyFlags(CPF_None)
			{
			}

			EPropertyFlags IncludePropertyFlags;
			EPropertyFlags ExcludePropertyFlags;
		};

		/**
		 * Recursively retrieve all nested object properties.
		 * Checks all root properties and properties of nested structs or exported objects.
		 * Does not handle maps or sets.
		 *
		 * @param InObject The object address to check. Generally can just provide a UObject instance.
		 * @param InPropertySource The source containing the properties. Usually your InObject->GetClass()
		 * @param OutProperties All found object properties.
		 * @param InArgs Additional configuration arguments.
		 */
		SMSYSTEMEDITOR_API void GetAllObjectProperties(const void* InObject, const UStruct* InPropertySource, TArray<FPropertyRetrieval>& OutProperties, const FPropertyRetrievalArgs& InArgs = FPropertyRetrievalArgs());

		/**
		 * Iterate over each valid instanced sub-object in an object. Sub-objects are matched to the owning object's
		 * ObjectProperty values where the owning property is marked Instanced but not transient.
		 *
		 * This is useful for cooking builds as sub-objects may have the transient flag added during cook when
		 * they normally wouldn't be a transient object.
		 *
		 * This won't account for properties belonging to a map or set.
		 *
		 * @param InObject The object owning the sub-objects. The sub-objects may be nested.
		 * @param Function A function to execute for each sub-object.
		 */
		SMSYSTEMEDITOR_API void ForEachInstancedSubObject(const UObject* InObject, TFunction<void(UObject*)> Function);

	}
}