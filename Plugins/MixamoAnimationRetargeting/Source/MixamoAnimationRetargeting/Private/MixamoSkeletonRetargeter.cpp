// Copyright 2022 UNAmedia. All Rights Reserved.

#include "MixamoSkeletonRetargeter.h"
#include "MixamoToolkitPrivatePCH.h"

#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"

#include "IKRigDefinition.h"
#include "RigEditor/IKRigController.h"
#include "RigEditor/IKRigDefinitionFactory.h"
#include "RetargetEditor/IKRetargeterController.h"
#include "RetargetEditor/IKRetargetFactory.h"
#include "Solvers/IKRig_PBIKSolver.h"

#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshLODModel.h"

#include "PackageTools.h"

#include "ARFilter.h"
#include "AssetRegistryModule.h"
#include "ScopedTransaction.h"
#include "Misc/ScopedSlowTask.h"

#include "SkeletonPoser.h"
#include "SkeletonMatcher.h"

#include "Editor.h"
#include "SMixamoToolkitWidget.h"
#include "ComponentReregisterContext.h"
#include "Components/SkinnedMeshComponent.h"



#define LOCTEXT_NAMESPACE "FMixamoAnimationRetargetingModule"


// Define it to ignore the UE5 mannequin as a valid retarget source
//#define MAR_IGNORE_UE5_MANNEQUIN
#ifdef MAR_IGNORE_UE5_MANNEQUIN
#	pragma message ("***WARNING*** Feature \"MetaHuman\" disabled.")
#endif


// Define it to disable the automatic addition of the Root Bone (needed to support UE4 Root Animations).
//#define MAR_ADDROOTBONE_DISABLE_
#ifdef MAR_ADDROOTBONE_DISABLE_
#	pragma message ("***WARNING*** Feature \"AddRootBone\" disabled.")
#endif

// Define it to disable the advance chains setup of the IK Retarger assets
//#define MAR_IKRETARGETER_ADVANCED_CHAINS_DISABLE_
#ifdef MAR_IKRETARGETER_ADVANCED_CHAINS_DISABLE_
#	pragma message ("***WARNING*** IK Retargeter is not using the advanced chains setup.")
#endif

// Define it to disable the IK solvers setup of the IK Retarger assets
//#define MAR_IKRETARGETER_IKSOLVERS_DISABLE_
#ifdef MAR_IKRETARGETER_IKSOLVERS_DISABLE_
#	pragma message ("***WARNING*** IK Retargeter is not using the IK solvers setup.")
#endif

//#define MAR_UPPERARMS_PRESERVECS_EXPERIMENTAL_ENABLE_
#ifdef MAR_UPPERARMS_PRESERVECS_EXPERIMENTAL_ENABLE_
#	pragma message ("***WARNING*** Preserving the Component Space bones of the upper arm bones is an experimental feature.")
#endif



namespace
{



/**
Index of the last Mixamo bone, in kUE4MannequinToMixamo_BoneNamesMapping arrays,
used to determine if a skeleton is from Mixamo.

Given the pair N-th, then index i = N * 2 + 1.
*/
constexpr int IndexLastCheckedMixamoBone = 22 * 2 + 1;
/**
Index of the last UE Mannequin bone, in kUE4MannequinToMixamo_BoneNamesMapping arrays,
used to determine if a skeleton is the UE Mannequin.

Given the pair N-th, then index i = N * 2 + 1.
*/
constexpr int IndexLastCheckedUEMannequinBone = 22 * 2;



/**
Mapping of "UE4 Mannequin" skeleton bones to the corresponding "Mixamo" skeleton bones names.

NOTES:
- includes the added "root" bone (by default it's missing in Mixamo skeletons and it's added by the plugin).
- the first N pairs [ N = (IndexLastCheckedMixamoBone + 1) / 2 ] are used to
	determine if a skeleton is from Mixamo.
*/
static const char* const kUE4MannequinToMixamo_BoneNamesMapping[] = {
	// UE Mannequin bone name		MIXAMO bone name
	"root",					"root",
	"pelvis", 				"Hips",
	"spine_01", 			"Spine",
	"spine_02", 			"Spine1",
	"spine_03", 			"Spine2",
	"neck_01", 				"Neck",
	"head", 				"head",
	"clavicle_l", 			"LeftShoulder",
	"upperarm_l", 			"LeftArm",
	"lowerarm_l", 			"LeftForeArm",
	"hand_l", 				"LeftHand",
	"clavicle_r", 			"RightShoulder",
	"upperarm_r", 			"RightArm",
	"lowerarm_r", 			"RightForeArm",
	"hand_r", 				"RightHand",
	"thigh_l", 				"LeftUpLeg",
	"calf_l", 				"LeftLeg",
	"foot_l", 				"LeftFoot",
	"ball_l", 				"LeftToeBase",
	"thigh_r", 				"RightUpLeg",
	"calf_r",				"RightLeg",
	"foot_r", 				"RightFoot",
	"ball_r", 				"RightToeBase",
	// From here, ignored to determine if a skeleton is from Mixamo.
	// From here, ignored to determine if a skeleton is from UE Mannequin.
	"index_01_l", 			"LeftHandIndex1",
	"index_02_l",			"LeftHandIndex2",
	"index_03_l", 			"LeftHandIndex3",
	"middle_01_l", 			"LeftHandMiddle1",
	"middle_02_l", 			"LeftHandMiddle2",
	"middle_03_l", 			"LeftHandMiddle3",
	"pinky_01_l", 			"LeftHandPinky1",
	"pinky_02_l", 			"LeftHandPinky2",
	"pinky_03_l", 			"LeftHandPinky3",
	"ring_01_l", 			"LeftHandRing1",
	"ring_02_l", 			"LeftHandRing2",
	"ring_03_l", 			"LeftHandRing3",
	"thumb_01_l", 			"LeftHandThumb1",
	"thumb_02_l", 			"LeftHandThumb2",
	"thumb_03_l", 			"LeftHandThumb3",
	"index_01_r", 			"RightHandIndex1",
	"index_02_r", 			"RightHandIndex2",
	"index_03_r", 			"RightHandIndex3",
	"middle_01_r", 			"RightHandMiddle1",
	"middle_02_r", 			"RightHandMiddle2",
	"middle_03_r", 			"RightHandMiddle3",
	"pinky_01_r", 			"RightHandPinky1",
	"pinky_02_r", 			"RightHandPinky2",
	"pinky_03_r", 			"RightHandPinky3",
	"ring_01_r", 			"RightHandRing1",
	"ring_02_r", 			"RightHandRing2",
	"ring_03_r", 			"RightHandRing3",
	"thumb_01_r", 			"RightHandThumb1",
	"thumb_02_r", 			"RightHandThumb2",
	"thumb_03_r", 			"RightHandThumb3",
	// Un-mapped bones (at the moment). Here for reference.
	//"lowerarm_twist_01_l", 	nullptr,
	//"upperarm_twist_01_l", 	nullptr,
	//"lowerarm_twist_01_r", 	nullptr,
	//"upperarm_twist_01_r", 	nullptr,
	//"calf_twist_01_l", 		nullptr,
	//"thigh_twist_01_l", 	nullptr,
	//"calf_twist_01_r", 		nullptr,
	//"thigh_twist_01_r", 	nullptr,
	//"ik_foot_root",			nullptr,
	//"ik_foot_l",			nullptr,
	//"ik_foot_r",			nullptr,
	//"ik_hand_root",			nullptr,
	//"ik_hand_gun",			nullptr,
	//"ik_hand_l",			nullptr,
	//"ik_hand_r",			nullptr,
};

constexpr int32 kUE4MannequinToMixamo_BoneNamesMapping_Num = sizeof(kUE4MannequinToMixamo_BoneNamesMapping) / sizeof(decltype(kUE4MannequinToMixamo_BoneNamesMapping[0]));
static_assert (kUE4MannequinToMixamo_BoneNamesMapping_Num % 2 == 0, "An event number of entries is expected");

static_assert (IndexLastCheckedMixamoBone % 2 == 1, "Mixamo indexes are odd numbers");
static_assert (IndexLastCheckedMixamoBone >= 1, "First valid Mixamo index is 1");
static_assert (IndexLastCheckedMixamoBone < kUE4MannequinToMixamo_BoneNamesMapping_Num, "Index out of bounds");

static_assert (IndexLastCheckedUEMannequinBone % 2 == 0, "UE Mannequin indexes are even numbers");
static_assert (IndexLastCheckedUEMannequinBone >= 0, "First valid UE Mannequin index is 0");
static_assert (IndexLastCheckedUEMannequinBone < kUE4MannequinToMixamo_BoneNamesMapping_Num, "Index out of bounds");



// UE5 mannequin bones in addition of the old mannequin.
// not all additional bones were included here (e.g fingers, etc)
static const char* const kUE5MannequinAdditionalBones[] =
{
	"spine_04",
	"spine_05",
	"neck_02",
	"lowerarm_twist_02_l",
	"lowerarm_twist_02_r",
	"upperarm_twist_02_l",
	"upperarm_twist_02_r",
	"thigh_twist_02_l",
	"thigh_twist_02_r",
	"calf_twist_02_l",
	"calf_twist_02_r"
};

/**
Mapping of "UE5 Mannequin" skeleton bones to the corresponding "Mixamo" skeleton bones names.

NOTES:
- includes the added "root" bone (by default it's missing in Mixamo skeletons and it's added by the plugin).
*/
static const char* const kUE5MannequinToMixamo_BoneNamesMapping[] = {
	// UE Mannequin bone name		MIXAMO bone name
	"root",					"root",
	"pelvis", 				"Hips",
	//"spine_01", 	nullptr,
	"spine_02", 			"Spine",
	"spine_03", 			"Spine1",
	"spine_04", 			"Spine2",
	//"spine_05", 	nullptr,
	"neck_01", 				"Neck",
	//"neck_02", 				nullptr,
	"head", 				"head",
	"clavicle_l", 			"LeftShoulder",
	"upperarm_l", 			"LeftArm",
	"lowerarm_l", 			"LeftForeArm",
	"hand_l", 				"LeftHand",
	"clavicle_r", 			"RightShoulder",
	"upperarm_r", 			"RightArm",
	"lowerarm_r", 			"RightForeArm",
	"hand_r", 				"RightHand",
	"thigh_l", 				"LeftUpLeg",
	"calf_l", 				"LeftLeg",
	"foot_l", 				"LeftFoot",
	"ball_l", 				"LeftToeBase",
	"thigh_r", 				"RightUpLeg",
	"calf_r",				"RightLeg",
	"foot_r", 				"RightFoot",
	"ball_r", 				"RightToeBase",
	"index_01_l", 			"LeftHandIndex1",
	"index_02_l",			"LeftHandIndex2",
	"index_03_l", 			"LeftHandIndex3",
	"middle_01_l", 			"LeftHandMiddle1",
	"middle_02_l", 			"LeftHandMiddle2",
	"middle_03_l", 			"LeftHandMiddle3",
	"pinky_01_l", 			"LeftHandPinky1",
	"pinky_02_l", 			"LeftHandPinky2",
	"pinky_03_l", 			"LeftHandPinky3",
	"ring_01_l", 			"LeftHandRing1",
	"ring_02_l", 			"LeftHandRing2",
	"ring_03_l", 			"LeftHandRing3",
	"thumb_01_l", 			"LeftHandThumb1",
	"thumb_02_l", 			"LeftHandThumb2",
	"thumb_03_l", 			"LeftHandThumb3",
	"index_01_r", 			"RightHandIndex1",
	"index_02_r", 			"RightHandIndex2",
	"index_03_r", 			"RightHandIndex3",
	"middle_01_r", 			"RightHandMiddle1",
	"middle_02_r", 			"RightHandMiddle2",
	"middle_03_r", 			"RightHandMiddle3",
	"pinky_01_r", 			"RightHandPinky1",
	"pinky_02_r", 			"RightHandPinky2",
	"pinky_03_r", 			"RightHandPinky3",
	"ring_01_r", 			"RightHandRing1",
	"ring_02_r", 			"RightHandRing2",
	"ring_03_r", 			"RightHandRing3",
	"thumb_01_r", 			"RightHandThumb1",
	"thumb_02_r", 			"RightHandThumb2",
	"thumb_03_r", 			"RightHandThumb3",
	// Un-mapped bones (at the moment). Here for reference.
	//"lowerarm_twist_01_l", 	nullptr,
	//"upperarm_twist_01_l", 	nullptr,
	//"lowerarm_twist_01_r", 	nullptr,
	//"upperarm_twist_01_r", 	nullptr,
	//"calf_twist_01_l", 		nullptr,
	//"thigh_twist_01_l", 	nullptr,
	//"calf_twist_01_r", 		nullptr,
	//"thigh_twist_01_r", 	nullptr,
	//"ik_foot_root",			nullptr,
	//"ik_foot_l",			nullptr,
	//"ik_foot_r",			nullptr,
	//"ik_hand_root",			nullptr,
	//"ik_hand_gun",			nullptr,
	//"ik_hand_l",			nullptr,
	//"ik_hand_r",			nullptr,
};

constexpr int32 kUE5MannequinToMixamo_BoneNamesMapping_Num = sizeof(kUE5MannequinToMixamo_BoneNamesMapping) / sizeof(decltype(kUE5MannequinToMixamo_BoneNamesMapping[0]));
static_assert (kUE5MannequinToMixamo_BoneNamesMapping_Num % 2 == 0, "An event number of entries is expected");



/**
Names of bones in the Mixamo skeleton that must preserve their Component Space transform (relative to the parent)
when re-posed to match the UE Mannequin skeleton base pose.
*/
static const TArray<FName> Mixamo_PreserveComponentSpacePose_BoneNames = {
	"Head",
	"LeftToeBase",
	"RightToeBase"

#ifdef MAR_UPPERARMS_PRESERVECS_EXPERIMENTAL_ENABLE_
	,"RightShoulder"
	,"RightArm"
	,"LeftShoulder"
	,"LeftArm"
#endif
};

/**
Names of bones in the UE4/UE5/MetaHuman Mannequin skeleton that must preserve their Component Space transform (relative to the parent)
when re-posed to match the Mixamo skeleton base pose.
*/
static const TArray<FName> UEMannequin_PreserveComponentSpacePose_BoneNames = {
	"head",
	"neck_02",	// This is used only by UE5/MetaHuman skeletons. UE4 doens't have it so it should be simply ignored.
	"ball_r",
	"ball_l"

#ifdef MAR_UPPERARMS_PRESERVECS_EXPERIMENTAL_ENABLE_
	,"clavicle_r"
	,"upperarm_r"
	,"clavicle_l"
	,"upperarm_l"
#endif
};



/**
Parent-child pair bone's names in the Mixamo skeleton where the child bone must be forcefully oriented 
when re-posed to match the UE Mannequin skeleton base pose regardless of the children number of the parent bone.
*/
static const TArray<TPair<FName, FName>> Mixamo_ParentChildBoneNamesToBypassOneChildConstraint = {
	{"LeftUpLeg", "LeftLeg"},
	{"LeftLeg", "LeftFoot"},
	{"LeftFoot", "LeftToeBase"},
	{"LeftToeBase", "LeftToe_End"},
	{"RightUpLeg", "RightLeg"},
	{"RightLeg", "RightFoot"},
	{"RightFoot", "RightToeBase"},
	{"RightToeBase", "RightToe_End"},
	{"Hips", "Spine"},	// Heuristic to try to align better the part.
	{"Spine", "Spine1"},
	{"Spine1", "Spine2"},
	{"Spine2", "Neck"},	// Heuristic to try to align better the part.
	{"Neck", "Head"},
	{"Head", "HeadTop_End"},
	{"LeftShoulder", "LeftArm"},
	{"LeftArm", "LeftForeArm"},
	{"LeftForeArm", "LeftHand"},
	{"LeftHand", "LeftHandMiddle1"},	// Heuristic to try to align better the part.
	{"LeftHandIndex1", "LeftHandIndex2"},
	{"LeftHandIndex2", "LeftHandIndex3"},
	{"LeftHandIndex3", "LeftHandIndex4"},
	{"LeftHandMiddle1", "LeftHandMiddle2"},
	{"LeftHandMiddle2", "LeftHandMiddle3"},
	{"LeftHandMiddle3", "LeftHandMiddle4"},
	{"LeftHandPinky1", "LeftHandPinky2"},
	{"LeftHandPinky2", "LeftHandPinky3"},
	{"LeftHandPinky3", "LeftHandPinky4"},
	{"LeftHandRing1", "LeftHandRing2"},
	{"LeftHandRing2", "LeftHandRing3"},
	{"LeftHandRing3", "LeftHandRing4"},
	{"LeftHandThumb1", "LeftHandThumb2"},
	{"LeftHandThumb2", "LeftHandThumb3"},
	{"LeftHandThumb3", "LeftHandThumb4"},
	{"RightShoulder", "RightArm"},
	{"RightArm", "RightForeArm"},
	{"RightForeArm", "RightHand"},
	{"RightHand", "RightHandMiddle1"},	// Heuristic to try to align better the part.
	{"RightHandIndex1", "RightHandIndex2"},
	{"RightHandIndex2", "RightHandIndex3"},
	{"RightHandIndex3", "RightHandIndex4"},
	{"RightHandMiddle1", "RightHandMiddle2"},
	{"RightHandMiddle2", "RightHandMiddle3"},
	{"RightHandMiddle3", "RightHandMiddle4"},
	{"RightHandPinky1", "RightHandPinky2"},
	{"RightHandPinky2", "RightHandPinky3"},
	{"RightHandPinky3", "RightHandPinky4"},
	{"RightHandRing1", "RightHandRing2"},
	{"RightHandRing2", "RightHandRing3"},
	{"RightHandRing3", "RightHandRing4"},
	{"RightHandThumb1", "RightHandThumb2"},
	{"RightHandThumb2", "RightHandThumb3"},
	{"RightHandThumb3", "RightHandThumb4"}
};

/**
Parent-child pair bone's names in the UE4 Mannequin skeleton where the child bone must be forcefully oriented
when re-posed to match the Mixamo skeleton base pose regardless of the children number of the parent bone.
*/
static const TArray<TPair<FName, FName>> UE4Mannequin_ParentChildBoneNamesToBypassOneChildConstraint = {
	{"pelvis", "spine_01"},	// Heuristic to try to align better the part.
	{"spine_01", "spine_02"},
	{"spine_02", "spine_03"},
	{"spine_03", "neck_01"},
	{"neck_01", "head"},
	{"thigh_l", "calf_l"},	// to ignore "thigh_twist_01_l"
	{"calf_l", "foot_l"},	// to ignore "calf_twist_01_l"
	{"foot_l", "ball_l"},
	{"thigh_r", "calf_r"},	// to ignore "thigh_twist_01_r"
	{"calf_r", "foot_r"},	// to ignore "calf_twist_01_r"
	{"foot_r", "ball_r"},
	{"clavicle_l", "upperarm_l"},
	{"upperarm_l", "lowerarm_l"},	// to ignore "upperarm_twist_01_l"
	{"lowerarm_l", "hand_l"},	// to ignore "lowerarm_twist_01_l"
	{"hand_l", "middle_01_l"},	// Heuristic to try to align better the part.
	{"index_01_l", "index_02_l"},
	{"index_02_l", "index_03_l"},
	{"middle_01_l", "middle_02_l"},
	{"middle_02_l", "middle_03_l"},
	{"pinky_01_l", "pinky_02_l"},
	{"pinky_02_l", "pinky_03_l"},
	{"ring_01_l", "ring_02_l"},
	{"ring_02_l", "ring_03_l"},
	{"thumb_01_l", "thumb_02_l"},
	{"thumb_02_l", "thumb_03_l"},
	{"clavicle_r", "upperarm_r"},
	{"upperarm_r", "lowerarm_r"},	// to ignore "upperarm_twist_01_r"
	{"lowerarm_r", "hand_r"},	// to ignore "lowerarm_twist_01_r"
	{"hand_r", "middle_01_r"},	// Heuristic to try to align better the part.
	{"index_01_r", "index_02_r"},
	{"index_02_r", "index_03_r"},
	{"middle_01_r", "middle_02_r"},
	{"middle_02_r", "middle_03_r"},
	{"pinky_01_r", "pinky_02_r"},
	{"pinky_02_r", "pinky_03_r"},
	{"ring_01_r", "ring_02_r"},
	{"ring_02_r", "ring_03_r"},
	{"thumb_01_r", "thumb_02_r"},
	{"thumb_02_r", "thumb_03_r"}
};

/**
Parent-child pair bone's names in the UE5/MetaHuman Mannequin skeleton where the child bone must be forcefully oriented
when re-posed to match the Mixamo skeleton base pose regardless of the children number of the parent bone.
*/
static const TArray<TPair<FName, FName>> UE5Mannequin_ParentChildBoneNamesToBypassOneChildConstraint = {
	{"pelvis", "spine_01"},	// Heuristic to try to align better the part.
	{"spine_01", "spine_02"},
	{"spine_02", "spine_03"},
	{"spine_03", "spine_04"},
	{"spine_04", "spine_05"},
	{"spine_05", "neck_01"},	// Heuristic to try to align better the part.
	{"neck_01", "neck_02"},
	{"neck_02", "head"},
	{"thigh_l", "calf_l"},
	{"calf_l", "foot_l"},
	{"foot_l", "ball_l"},
	{"thigh_r", "calf_r"},
	{"calf_r", "foot_r"},
	{"foot_r", "ball_r"},
	{"clavicle_l", "upperarm_l"},
	{"upperarm_l", "lowerarm_l"},
	{"lowerarm_l", "hand_l"},
	{"hand_l", "middle_metacarpal_l"},	// Heuristic to try to align better the part.
	{"index_metacarpal_l", "index_01_l"},
	{"index_01_l", "index_02_l"},
	{"index_02_l", "index_03_l"},
	{"middle_metacarpal_l", "middle_01_l"},
	{"middle_01_l", "middle_02_l"},
	{"middle_02_l", "middle_03_l"},
	{"pinky_metacarpal_l", "pinky_01_l"},
	{"pinky_01_l", "pinky_02_l"},
	{"pinky_02_l", "pinky_03_l"},
	{"ring_metacarpal_l", "ring_01_l"},
	{"ring_01_l", "ring_02_l"},
	{"ring_02_l", "ring_03_l"},
	{"thumb_01_l", "thumb_02_l"},
	{"thumb_02_l", "thumb_03_l"},
	{"clavicle_r", "upperarm_r"},
	{"upperarm_r", "lowerarm_r"},
	{"lowerarm_r", "hand_r"},
	{"hand_r", "middle_metacarpal_r"},	// Heuristic to try to align better the part.
	{"index_metacarpal_r", "index_01_r"},
	{"index_01_r", "index_02_r"},
	{"index_02_r", "index_03_r"},
	{"middle_metacarpal_r", "middle_01_r"},
	{"middle_01_r", "middle_02_r"},
	{"middle_02_r", "middle_03_r"},
	{"pinky_metacarpal_r", "pinky_01_r"},
	{"pinky_01_r", "pinky_02_r"},
	{"pinky_02_r", "pinky_03_r"},
	{"ring_metacarpal_r", "ring_01_r"},
	{"ring_01_r", "ring_02_r"},
	{"ring_02_r", "ring_03_r"},
	{"thumb_01_r", "thumb_02_r"},
	{"thumb_02_r", "thumb_03_r"}
};

static const FName RootBoneName("root");



#ifndef MAR_IKRETARGETER_ADVANCED_CHAINS_DISABLE_
/**
Mapping of "UE4/UE5/MetaHuman Mannequin" chain names to the corresponding "Mixamo" chain names.
*/
static const char* const kUEMannequinToMixamo_ChainNamesMapping[] = {
	"Root", "Root",
	"Spine", "Spine",
	"Head", "Head",
	"LeftClavicle", "LeftClavicle",
	"RightClavicle", "RightClavicle",
	"LeftArm", "LeftArm",
	"RightArm", "RightArm",
	"LeftLeg", "LeftLeg",
	"RightLeg", "RightLeg",
	"LeftIndex", "LeftIndex",
	"RightIndex", "RightIndex",
	"LeftMiddle", "LeftMiddle",
	"RightMiddle", "RightMiddle",
	"LeftPinky", "LeftPinky",
	"RightPinky", "RightPinky",
	"LeftRing", "LeftRing",
	"RightRing", "RightRing",
	"LeftThumb", "LeftThumb",
	"RightThumb", "RightThumb"
};

constexpr int32 kUEMannequinToMixamo_ChainNamesMapping_Num = sizeof(kUEMannequinToMixamo_ChainNamesMapping) / sizeof(decltype(kUEMannequinToMixamo_ChainNamesMapping[0]));
static_assert (kUEMannequinToMixamo_ChainNamesMapping_Num % 2 == 0, "An event number of entries is expected");
#endif


/**
List of "chain names" (relative to the UE4/UE5/MetaHuman Mannequin names) that must not be configured in the IKRetarget asset.
*/
static const TArray<FName> UEMannequin_SkipChains_ChainNames = {
#ifdef MAR_IKRETARGETER_ADVANCED_CHAINS_DISABLE_
	TEXT("root"),
	TEXT("pelvis")
#endif
};


/**
List of "chain names" (relative to the UE4/UE5/MetaHuman Mannequin names) for which the "Drive IK Goal" must be configured.
*/
static const TArray<FName> UEMannequin_DriveIKGoal_ChainNames = {
	"LeftArm",
	"RightArm",
	"LeftLeg",
	"RightLeg"
};


/**
List of "chain names" (relative to the UE4/UE5/MetaHuman Mannequin names) for which the "one to one" must be set as FK Rotation mode.
*/
static const TArray<FName> UEMannequin_OneToOneFKRotationMode_ChainNames = {
	"LeftIndex",
	"RightIndex",
	"LeftMiddle",
	"RightMiddle",
	"LeftPinky",
	"RightPinky",
	"LeftRing",
	"RightRing",
	"LeftThumb",
	"RightThumb",
};



#ifndef MAR_IGNORE_UE5_MANNEQUIN

static const TCHAR* kMetaHumanBaseSkeleton_ObjectPath = TEXT("/Game/MetaHumans/Common/Female/Medium/NormalWeight/Body/metahuman_base_skel.metahuman_base_skel");
static const TCHAR* kMetaHumanDefaultSkeletalMesh_ObjectPath = TEXT("/Game/MetaHumans/Common/Female/Medium/NormalWeight/Body/f_med_nrw_body.f_med_nrw_body");

#endif // MAR_IGNORE_UE5_MANNEQUIN



/// Returns the "cleaned" name of a skeleton asset.
FString GetBaseSkeletonName(const USkeleton* Skeleton)
{
	check(Skeleton != nullptr);
	FString Name = Skeleton->GetName();
	Name.RemoveFromStart(TEXT("SK_"));
	Name.RemoveFromEnd(TEXT("Skeleton"));
	Name.RemoveFromEnd(TEXT("_"));
	return Name;
}

/// Returns a nicer name for the IKRig asset associated to Skeleton.
FString GetRigName(const USkeleton* Skeleton)
{
	return FString::Printf(TEXT("IK_%s"), *GetBaseSkeletonName(Skeleton));
}


/// Returns a nicer name for the IKRetargeter asset used to retarget from ReferenceSkeleton to Skeleton.
FString GetRetargeterName(const USkeleton* ReferenceSkeleton, const USkeleton* Skeleton)
{
	return FString::Printf(TEXT("RTG_%s_%s"), *GetBaseSkeletonName(ReferenceSkeleton), *GetBaseSkeletonName(Skeleton));
}



// See FSkeletonPoser::ComputeComponentSpaceTransform().
// TODO: this is used by ConfigureBoneLimitsLocalToBS(); if we'll keep this function, it could be helpful to make the above one public and use it.
FTransform ComputeComponentSpaceTransform(const FReferenceSkeleton & RefSkeleton, int32 BoneIndex)
{
	if (BoneIndex == INDEX_NONE)
	{
		return FTransform::Identity;
	}

	const TArray<FTransform>& RelTransforms = RefSkeleton.GetRefBonePose();
	FTransform T = RelTransforms[BoneIndex];
	int32 i = RefSkeleton.GetParentIndex(BoneIndex);
	while (i != INDEX_NONE)
	{
		T *= RelTransforms[i];
		i = RefSkeleton.GetParentIndex(i);
	}

	return T;
}



/**
Configure the bone preferred angle converting from the input "Local Space" to the Skeleton Bone Space.

Local space is constructed with the forward vector pointing to the bone direction (the direction pointing the child bone), 
the input right vector (BoneLimitRightCS) and the Up vector as the cross product of the two.

Preferred angles are then remapped from these axis to the **matching** skeleton bone space axis.

Bone name is in Settings.
*/
void ConfigureBonePreferredAnglesLocalToBS(
	const USkeleton* Skeleton,
	UIKRig_PBIKBoneSettings* Settings,
	FName ChildBoneName,
	FVector PreferredAnglesLS,
	FVector BoneLimitRightCS
)
{
	check(Skeleton != nullptr);
	if (Settings == nullptr)
	{
		return;
	}
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	const int32 BoneIndex = RefSkeleton.FindBoneIndex(Settings->Bone);
	// Skip if required data are missing.
	if (BoneIndex == INDEX_NONE)
	{
		return;
	}

	const int32 ChildBoneIndex = RefSkeleton.FindBoneIndex(ChildBoneName);
	if (ChildBoneIndex == INDEX_NONE)
	{
		return;
	}

	check(RefSkeleton.GetParentIndex(ChildBoneIndex) == BoneIndex);

	auto GetMatchingAxis = [](const TArray<FVector>& axis, const FVector& refAxis, bool& bInverted)
	{
		float ProjOnDir = FVector::DotProduct(axis[0], refAxis);
		int bestIdx = 0;
		for(int i = 1; i < 3; ++i)
		{
			float dot = FVector::DotProduct(axis[i], refAxis);
			if (FMath::Abs(dot) > FMath::Abs(ProjOnDir))
			{
				ProjOnDir = dot;
				bestIdx = i;
			}
		}

		bInverted = ProjOnDir < 0;
		return bestIdx;
	};

	const FTransform BoneCS = ComputeComponentSpaceTransform(RefSkeleton, BoneIndex);
	const FTransform ChildBoneCS = ComputeComponentSpaceTransform(RefSkeleton, ChildBoneIndex);
	const FQuat& BoneR = BoneCS.GetRotation();

	const FVector XAxisCS = BoneCS.GetUnitAxis(EAxis::X);
	const FVector YAxisCS = BoneCS.GetUnitAxis(EAxis::Y);
	const FVector ZAxisCS = BoneCS.GetUnitAxis(EAxis::Z);

	const FVector ParentToChildDirCS = (ChildBoneCS.GetTranslation() - BoneCS.GetTranslation()).GetSafeNormal();

	// NOTE: [XBoneLimitCS, YBoneLimitCS, ZBoneLimitCS] could be NOT an orthonormal basis!
	const FVector XBoneLimitCS = ParentToChildDirCS;
	const FVector YBoneLimitCS = BoneLimitRightCS;
	const FVector ZBoneLimitCS = FVector::CrossProduct(XBoneLimitCS, YBoneLimitCS);
	check(!ZBoneLimitCS.IsNearlyZero());

	bool bInverted[3];
	int remappedAxis[3];

	remappedAxis[0] = GetMatchingAxis({ XAxisCS , YAxisCS , ZAxisCS }, XBoneLimitCS, bInverted[0]);
	remappedAxis[1] = GetMatchingAxis({ XAxisCS , YAxisCS , ZAxisCS }, YBoneLimitCS, bInverted[1]);
	remappedAxis[2] = GetMatchingAxis({ XAxisCS , YAxisCS , ZAxisCS }, ZBoneLimitCS, bInverted[2]);

	for (int i = 0; i < 3; ++i)
	{
		int axisIdx = remappedAxis[i];
		float angle = PreferredAnglesLS[i];
		float sign = 1.0f;

		if (bInverted[i])
			sign = -1.0f;
		if(((i == 2) ^ (axisIdx == 2)) == true)
			sign *= -1.0f;

		Settings->PreferredAngles[axisIdx] = angle * sign;
	}

	Settings->bUsePreferredAngles = true;
}



// A #define since I don't want to copy buffers/objects, and doing it with C++ template specialization is a mess...
#define SelectBySkeletonType(SkeletonType,UE4Value,UE5Value)	((SkeletonType) == ETargetSkeletonType::ST_UE5_MANNEQUIN ? UE5Value : UE4Value)

} // namespace *unnamed*



FMixamoSkeletonRetargeter::FMixamoSkeletonRetargeter()
	: UE4MannequinToMixamo_BoneNamesMapping(kUE4MannequinToMixamo_BoneNamesMapping, kUE4MannequinToMixamo_BoneNamesMapping_Num),
#ifdef MAR_IKRETARGETER_ADVANCED_CHAINS_DISABLE_
	  UE4MannequinToMixamo_ChainNamesMapping(kUE4MannequinToMixamo_BoneNamesMapping, kUE4MannequinToMixamo_BoneNamesMapping_Num),
#else
	  UE4MannequinToMixamo_ChainNamesMapping(kUEMannequinToMixamo_ChainNamesMapping, kUEMannequinToMixamo_ChainNamesMapping_Num),
#endif
	  UE5MannequinToMixamo_BoneNamesMapping(kUE5MannequinToMixamo_BoneNamesMapping, kUE5MannequinToMixamo_BoneNamesMapping_Num),
#ifdef MAR_IKRETARGETER_ADVANCED_CHAINS_DISABLE_
	  UE5MannequinToMixamo_ChainNamesMapping(kUE5MannequinToMixamo_BoneNamesMapping, kUE5MannequinToMixamo_BoneNamesMapping_Num)
#else
	  UE5MannequinToMixamo_ChainNamesMapping(kUEMannequinToMixamo_ChainNamesMapping, kUEMannequinToMixamo_ChainNamesMapping_Num)
#endif
{
}



/**
Retarget all the Skeletons (Mixamo skeletons) to a UE Mannequin skeleton that the user will interactively select.
*/
void FMixamoSkeletonRetargeter::RetargetToUE4Mannequin(TArray<USkeleton *> Skeletons) const
{
	if (Skeletons.Num() <= 0)
	{
		return;
	}

	// Get the UE4 "Mannequin" skeleton.
	USkeleton * ReferenceSkeleton = AskUserForTargetSkeleton();
	if (ReferenceSkeleton == nullptr)
	{
		// We hadn't found a suitable skeleton.
		FMessageLog("LogMixamoToolkit").Error(FText::FromString(TEXT("No suitable Skeleton selected. Retargeting aborted.")));
		return;
	}

	TArray<UObject*> AssetsToOverwrite;
	for (USkeleton* Skeleton : Skeletons)
	{
		EnumerateAssetsToOverwrite(Skeleton, ReferenceSkeleton, AssetsToOverwrite);
	}
	if (AssetsToOverwrite.Num() && !AskUserOverridingAssetsConfirmation(AssetsToOverwrite))
	{
		FMessageLog("LogMixamoToolkit").Error(FText::FromString(TEXT("Files overwritten denied. Retargeting aborted by the user.")));
		return;
	}

	// Ensure that the ReferenceSkeleton has a preview mesh!
	// Without it, retargeting an animation will fail (CreateUEMannequinIKRig -> CreateIKRig -> will be unable to get a required skeletal mesh)
	const ETargetSkeletonType ReferenceSkeletonType = GetTargetSkeletonType(ReferenceSkeleton);
	bool bSetPreviewMesh = ( ReferenceSkeleton->GetPreviewMesh() == nullptr );

#ifndef MAR_IGNORE_UE5_MANNEQUIN
	const bool bIsMetaHumanSkeleton = (
		ReferenceSkeletonType == ETargetSkeletonType::ST_UE5_MANNEQUIN
		&& ReferenceSkeleton->GetPathName().Equals(kMetaHumanBaseSkeleton_ObjectPath, ESearchCase::IgnoreCase)
	);

	// If targetting the MetaHuman skeleton ("metahuman_base_skel"), ensure it's
	// using the default skeletal mesh "f_med_nrw_body" as preview.
	//
	// Code in Retarget() will process only "f_med_nrw_body" and will filter out all
	// the other skeletal meshes; but later code relies on the Preview Mesh for some
	// computations resulting in an error if the returned skeletal mesh has not been
	// processed.
	// While it would be better to forcefully use "f_med_nrw_body" without changing
	// any Preview Mesh set by the user, for now this is the simplest solution to not
	// revolutionize existing code.
	if (bIsMetaHumanSkeleton)
	{
		bSetPreviewMesh = (
			ReferenceSkeleton->GetPreviewMesh() == nullptr
			|| !ReferenceSkeleton->GetPreviewMesh()->GetPathName().Equals(kMetaHumanDefaultSkeletalMesh_ObjectPath, ESearchCase::IgnoreCase)
		);
	}
#endif

	if (bSetPreviewMesh)
	{
		TArray<FAssetData> ReferenceSkeletalMeshes;
		GetAllSkeletalMeshesUsingSkeleton(ReferenceSkeleton, ReferenceSkeletalMeshes);

#ifndef MAR_IGNORE_UE5_MANNEQUIN
		// In case of a MetaHuman skeleton, try to pick a good preview mesh (move it at index 0).
		if (bIsMetaHumanSkeleton)
		{
			auto RepositionSkeletalMesheByObjectPath = [&](const FName& Query) {
				int32 i = ReferenceSkeletalMeshes.IndexOfByPredicate(
					[&](const FAssetData& A) {
						return A.ObjectPath == Query;
					}
				);
				if (i == INDEX_NONE)
				{
					return false;
				}
				ReferenceSkeletalMeshes.Swap(i, 0);
				return true;
			};

			if (!RepositionSkeletalMesheByObjectPath(kMetaHumanDefaultSkeletalMesh_ObjectPath))
			{
				const FString ErrorMessage = FString::Format(TEXT("Default MetaHuman skeletal mesh '{0}' not found. Retargeting aborted."),
					{ kMetaHumanDefaultSkeletalMesh_ObjectPath });

				//RepositionSkeletalMesheByObjectPath(TEXT("/Game/MetaHumans/Common/Male/Medium/NormalWeight/Body/m_med_nrw_body.m_med_nrw_body"));
				FMessageLog("LogMixamoToolkit").Error(FText::FromString(ErrorMessage));
				return;
			}
		}
#endif // MAR_IGNORE_UE5_MANNEQUIN

		// This will load the Skeletal Mesh.
		ReferenceSkeleton->SetPreviewMesh(CastChecked<USkeletalMesh>(ReferenceSkeletalMeshes[0].GetAsset()));
	}

	// Process all input skeletons.
	FScopedSlowTask Progress(Skeletons.Num(), LOCTEXT("FMixamoSkeletonRetargeter_ProgressTitle", "Retargeting of Mixamo assets"));
	Progress.MakeDialog();
	const FScopedTransaction Transaction(LOCTEXT("FMixamoSkeletonRetargeter_RetargetSkeletons", "Retargeting of Mixamo assets"));
	for (USkeleton * Skeleton : Skeletons)
	{
		Progress.EnterProgressFrame(1, FText::FromName(Skeleton->GetFName()));
		Retarget(Skeleton, ReferenceSkeleton, ReferenceSkeletonType);
	}
}



/// Return true if Skeleton is a Mixamo skeleton.
bool FMixamoSkeletonRetargeter::IsMixamoSkeleton(const USkeleton * Skeleton) const
{
	// We consider a Skeleton "coming from Mixamo" if it has at least X% of the expected bones.
	const float MINIMUM_MATCHING_PERCENTAGE = .75f;

	// Convert the array of expected bone names (TODO: cache it...).
	TArray<FName> BoneNames;
	UE4MannequinToMixamo_BoneNamesMapping.GetDestination(BoneNames);
	// Look for and count the known Mixamo bones (see comments on IndexLastCheckedMixamoBone and UEMannequinToMixamo_BonesMapping).
	constexpr int32 NumBones = (IndexLastCheckedMixamoBone + 1) / 2;
	BoneNames.SetNum(NumBones);
	
	FSkeletonMatcher SkeletonMatcher(BoneNames, MINIMUM_MATCHING_PERCENTAGE);
	return SkeletonMatcher.IsMatching(Skeleton);
}



/// Return true if AssetData is NOT a UE Mannequin skeleton asset.
bool FMixamoSkeletonRetargeter::OnShouldFilterNonUEMannequinSkeletonAsset(const FAssetData& AssetData) const
{
	// Skip non skeleton assets.
	if (!AssetData.IsInstanceOf(USkeleton::StaticClass()))
	{
		return false;
	}

#ifndef MAR_IGNORE_UE5_MANNEQUIN
	// Special filtering for skeletons in '/Game/MetaHumans/' path: inside this path,
	// we want to show only the "metahuman_base_skel" (corresponding to the Female-Medium-NormalWeight body).
	//
	// This because all the MetaHuman skeletal meshes are based on it (see <https://docs.metahuman.unrealengine.com/en-US/MetahumansUnrealEngine/MetaHumanRetargetAnimations/>),
	// so any other skeleton here can/must be ignored.
	//
	// In particular, downloaded accessories (tested with some characters manually downloaded from the Quixel Website)
	// can have skeletons compatible with "metahuman_base_skel" (in particular for clothes like "tops", e.g.
	// "/Game/MetaHumans/Common/Male/Tall/OverWeight/Tops/Hoodie/Meshes/m_tal_ovw_top_hoodie_Skeleton").
	// At run-time, their animation are instead driven by "metahuman_base_skel": in the MetaHuman actor blueprint
	// (e.g. "/Game/MetaHumans/Hudson/BP_Hudson"), in the Construction Script, the function "EnableMasterPose"
	// is called for all the skeletal mesh components forcing them to use the "Body" as Master Pose Component
	// (https://docs.unrealengine.com/5.0/en-US/modular-characters-in-unreal-engine/); this causes that all the animations
	// will be driven by the "Body" skeletal mesh that is configured to use "metahuman_base_skel".
	// Since they're not used at run-time for animations purposes, it's pointless to select them for retargeting.
	if (AssetData.PackagePath.ToString().StartsWith(TEXT("/Game/MetaHumans/"), ESearchCase::IgnoreCase))
	{
		return !AssetData.ObjectPath.IsEqual(kMetaHumanBaseSkeleton_ObjectPath);
	}
#endif

	// To check the skeleton bones, unfortunately we've to load the asset.
	USkeleton* Skeleton = Cast<USkeleton>(AssetData.GetAsset());
	return Skeleton != nullptr ? !IsUEMannequinSkeleton(Skeleton) : true;
}



ETargetSkeletonType FMixamoSkeletonRetargeter::GetTargetSkeletonType(const USkeleton* Skeleton) const
{
	// We consider a Skeleton "being the UE Mannequin" if it has at least X% of the expected bones.
	const float MINIMUM_MATCHING_PERCENTAGE = .75f;

	// Convert the array of expected bone names
	/// @TODO: cache it.
	TArray<FName> BoneNames;
	UE4MannequinToMixamo_BoneNamesMapping.GetSource(BoneNames);
	// Look for and count the known UE Mannequin bones (see comments on IndexLastCheckedUEMannequinBone and UEMannequinToMixamo_BonesMapping).
	constexpr int32 NumBones = (IndexLastCheckedUEMannequinBone + 2) / 2;
	BoneNames.SetNum(NumBones);
	
	FSkeletonMatcher SkeletonMatcher(BoneNames, MINIMUM_MATCHING_PERCENTAGE);
	if (SkeletonMatcher.IsMatching(Skeleton))
	{
		// It can be an UE4 or an UE5/MetaHuman skeleton, disambiguate it.
		TArray<FName> UE5BoneNames;
		for (int i = 0; i < sizeof(kUE5MannequinAdditionalBones) / sizeof(decltype(kUE5MannequinAdditionalBones[0])); ++i)
		{
			UE5BoneNames.Add(kUE5MannequinAdditionalBones[i]);
		}

		const float MINIMUM_MATCHING_PERCENTAGE_UE5 = .25f;
		FSkeletonMatcher SkeletonMatcherUE5(UE5BoneNames, MINIMUM_MATCHING_PERCENTAGE_UE5);

		return SkeletonMatcherUE5.IsMatching(Skeleton) ? ETargetSkeletonType::ST_UE5_MANNEQUIN : ETargetSkeletonType::ST_UE4_MANNEQUIN;
	}

	return ETargetSkeletonType::ST_UNKNOWN;
}



/// Return true if Skeleton is a UE Mannequin skeleton.
bool FMixamoSkeletonRetargeter::IsUEMannequinSkeleton(const USkeleton* Skeleton) const
{
	const ETargetSkeletonType Type = GetTargetSkeletonType(Skeleton);
	return
		Type == ETargetSkeletonType::ST_UE4_MANNEQUIN
#ifndef MAR_IGNORE_UE5_MANNEQUIN
		|| Type == ETargetSkeletonType::ST_UE5_MANNEQUIN
#endif
		;
}



/**
Process Skeleton to support retargeting to ReferenceSkeleton.

Usually this requires to process all the Skeletal Meshes based on Skeleton.
*/
void FMixamoSkeletonRetargeter::Retarget(
	USkeleton* Skeleton,
	const USkeleton * ReferenceSkeleton,
	ETargetSkeletonType ReferenceSkeletonType
) const
{
	check(Skeleton != nullptr);
	check(ReferenceSkeleton != nullptr);

	FMessageLog("LogMixamoToolkit").Info(FText::FromString(FString::Format(TEXT("Retargeting Mixamo skeleton '{0}'"),
		{ *Skeleton->GetName() })));

	// Check for a skeleton retargeting on itself.
	if (Skeleton == ReferenceSkeleton)
	{
		FMessageLog("LogMixamoToolkit").Warning(FText::FromString(FString::Format(TEXT("Skipping retargeting of Mixamo skeleton '{0}' on itself"),
			{ *Skeleton->GetName() })));
		return;
	}

	// Check for invalid root bone (root bone not at position 0)
	if (HasFakeRootBone(Skeleton))
	{
		FMessageLog("LogMixamoToolkit").Warning(FText::FromString(FString::Format(TEXT("Skipping retargeting of Mixamo skeleton '{0}'; invalid 'root' bone at index != 0"),
			{ *Skeleton->GetName() })));
		return;
	}

	// Get all USkeletalMesh assets using Skeleton (i.e. the Mixamo skeleton).
	TArray<USkeletalMesh *> SkeletalMeshes;
	GetAllSkeletalMeshesUsingSkeleton(Skeleton, SkeletalMeshes);

	/*
		Retargeting uses the SkeletalMesh's reference skeleton, as it counts for mesh proportions.
		If you need to use the original Skeleton, you have to ensure Skeleton pose has the same proportions
		of the skeletal mesh we are retargeting calling:
			Skeleton->GetPreviewMesh(true)->UpdateReferencePoseFromMesh(SkeletonMesh);
	*/
	// Add the root bone if needed. This: fixes a offset glitch in the animations, is generally useful.
#ifndef MAR_ADDROOTBONE_DISABLE_
	AddRootBone(Skeleton, SkeletalMeshes);
#endif

	// Be sure that the Skeleton has a preview mesh!
	// without it, retargeting an animation will fail
	if (SkeletalMeshes.Num() > 0)
		SetPreviewMesh(Skeleton, SkeletalMeshes[0]);

	// Create the IKRig assets, one for each input skeleton.
	UIKRigDefinition* MixamoRig = CreateMixamoIKRig(Skeleton);
	UIKRigDefinition* UEMannequinRig = CreateUEMannequinIKRig(ReferenceSkeleton, ReferenceSkeletonType);

	// Create the IKRetarget asset to retarget from the UE Mannequin to Mixamo.
	const FString SkeletonBasePackagePath = FPackageName::GetLongPackagePath(Skeleton->GetPackage()->GetName());
	const FStaticNamesMapper & UEMannequinToMixamo_ChainNamesMapping = SelectBySkeletonType(ReferenceSkeletonType, UE4MannequinToMixamo_ChainNamesMapping, UE5MannequinToMixamo_ChainNamesMapping);
	const FStaticNamesMapper MixamoToUEMannequin_ChainNamesMapping = UEMannequinToMixamo_ChainNamesMapping.GetInverseMapper();
	UIKRetargeter* IKRetargeter_UEMannequinToMixamo = CreateIKRetargeter(
		SkeletonBasePackagePath,
		GetRetargeterName(ReferenceSkeleton, Skeleton),
		UEMannequinRig,
		MixamoRig,
		MixamoToUEMannequin_ChainNamesMapping,
		UEMannequinToMixamo_ChainNamesMapping.MapNames(UEMannequin_SkipChains_ChainNames),
		UEMannequinToMixamo_ChainNamesMapping.MapNames(UEMannequin_DriveIKGoal_ChainNames),
		UEMannequinToMixamo_ChainNamesMapping.MapNames(UEMannequin_OneToOneFKRotationMode_ChainNames)
	);

	// Set-up the translation retargeting modes, to avoid artifacts when retargeting the animations.
	SetupTranslationRetargetingModes(Skeleton);
	// Retarget the base pose of the Mixamo skeletal meshes to match the "UE4_Mannequin_Skeleton" one.
	const FStaticNamesMapper & UEMannequinToMixamo_BoneNamesMapping = SelectBySkeletonType(ReferenceSkeletonType, UE4MannequinToMixamo_BoneNamesMapping, UE5MannequinToMixamo_BoneNamesMapping);
	RetargetBasePose(
		SkeletalMeshes,
		ReferenceSkeleton,
		Mixamo_PreserveComponentSpacePose_BoneNames,
		UEMannequinToMixamo_BoneNamesMapping.GetInverseMapper(),
		Mixamo_ParentChildBoneNamesToBypassOneChildConstraint,
		/*bApplyPoseToRetargetBasePose=*/true,
		UIKRetargeterController::GetController(IKRetargeter_UEMannequinToMixamo)
	);

	// = Setup the Mixamo to UE Mannequin retargeting.
	
	// Get all USkeletalMesh assets using ReferenceSkeleton (i.e. the UE Mannequin skeleton).
	TArray<USkeletalMesh*> UEMannequinSkeletalMeshes;
	GetAllSkeletalMeshesUsingSkeleton(ReferenceSkeleton, UEMannequinSkeletalMeshes);

#ifndef MAR_IGNORE_UE5_MANNEQUIN
	// In case Skeleton is a MetaHuman skeleton, remove all the skeletal meshes
	// but "/Game/MetaHumans/Common/Female/Medium/NormalWeight/Body/f_med_nrw_body'.
	//
	// This because the "f_med_nrw_body" skeletal mesh is the only one that creates
	// correct retargeted animations for all types of MetaHuman characters, also if
	// the target MetaHuman character is using a different configuration (!) (e.g.
	// Male-Tall-OverWeight, not sure how this works behind the scenes at the moment).
	// When try using a different skeletal mesh, also if matching the target character
	// configuration, the final animated character has evident rigging problems.
	// Forcing the user with the "f_med_nrw_body" option only, guarantees correct results
	// (provided the user preserves the corresponding preview skeletal mesh configured
	// by the plugin).
	if (ReferenceSkeletonType == ETargetSkeletonType::ST_UE5_MANNEQUIN)
	{
		UEMannequinSkeletalMeshes = UEMannequinSkeletalMeshes.FilterByPredicate([](USkeletalMesh* S) {
			const FString PathName = S->GetPathName();
			return
				!PathName.StartsWith("/Game/MetaHumans/", ESearchCase::IgnoreCase)
				|| PathName.Equals(kMetaHumanDefaultSkeletalMesh_ObjectPath, ESearchCase::IgnoreCase);
		});
	}
#endif // MAR_IGNORE_UE5_MANNEQUIN

	// Create the IKRetarget asset to retarget from Mixamo to the UE Mannequin.
	UIKRetargeter* IKRetargeter_MixamoToUEMannequin = CreateIKRetargeter(
		SkeletonBasePackagePath,
		GetRetargeterName(Skeleton, ReferenceSkeleton),
		MixamoRig,
		UEMannequinRig,
		UEMannequinToMixamo_ChainNamesMapping,
		UEMannequin_SkipChains_ChainNames,
		UEMannequin_DriveIKGoal_ChainNames,
		UEMannequin_OneToOneFKRotationMode_ChainNames
	);

	// Retarget the base pose of the UE Mannequin skeletal meshes to match the Mixamo skeleton one.
	// Only in the IK Retargeter asset, do not change the UE Mannueqin Skeletal Meshes.
	const TArray<TPair<FName, FName>> & UEMannequin_ParentChildBoneNamesToBypassOneChildConstraint = SelectBySkeletonType(ReferenceSkeletonType, UE4Mannequin_ParentChildBoneNamesToBypassOneChildConstraint, UE5Mannequin_ParentChildBoneNamesToBypassOneChildConstraint);
	RetargetBasePose(
		UEMannequinSkeletalMeshes,
		Skeleton,
		UEMannequin_PreserveComponentSpacePose_BoneNames,
		UEMannequinToMixamo_BoneNamesMapping,
		UEMannequin_ParentChildBoneNamesToBypassOneChildConstraint,
		/*bApplyPoseToRetargetBasePose=*/false,
		UIKRetargeterController::GetController(IKRetargeter_MixamoToUEMannequin)
	);

	/*
	// Open a content browser showing the specified assets (technically all the content of the directories containing them...).

	TArray<UObject*> ObjectsToSync;
	ObjectsToSync.Add(IKRetargeter_UEMannequinToMixamo);
	ObjectsToSync.Add(IKRetargeter_MixamoToUEMannequin);
	ObjectsToSync.Add(MixamoRig);
	ObjectsToSync.Add(UEMannequinRig);
	GEditor->SyncBrowserToObjects(ObjectsToSync);
	*/

	FMessageLog("LogMixamoToolkit").Info(FText::FromString(FString::Format(TEXT("Mixamo skeleton '{0}' retargeted successfully."),
		{ *Skeleton->GetName() })));
}



/// Return true if Skeleton has a bone named "root" and it's not at position 0; return false otherwise.
bool FMixamoSkeletonRetargeter::HasFakeRootBone(const USkeleton* Skeleton) const
{
	check(Skeleton != nullptr);
	const int32 rootBoneIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(RootBoneName);
	return rootBoneIndex != INDEX_NONE && rootBoneIndex != 0;
}



/// Add the "root" bone to Skeleton and all its SkeletalMeshes.
void FMixamoSkeletonRetargeter::AddRootBone(USkeleton* Skeleton, TArray<USkeletalMesh *> SkeletalMeshes) const
{
	// Skip if the mesh has already a bone named "root".
	if (Skeleton->GetReferenceSkeleton().FindBoneIndex(RootBoneName) != INDEX_NONE)
	{
		return;
	}

	//=== Add the root bone to all the Skeletal Meshes using Skeleton.
	// We'll have to fix the Skeletal Meshes to account for the added root bone.

	// When going out of scope, it'll re-register components with the scene.
	TComponentReregisterContext<USkinnedMeshComponent> ReregisterContext;

	// Add the root bone to *all* skeletal meshes in SkeletalMeshes.
	for (int iMesh = 0; iMesh < SkeletalMeshes.Num(); ++ iMesh)
	{
		USkeletalMesh * SkeletonMesh = SkeletalMeshes[iMesh];
		ensure(SkeletonMesh != nullptr);	// @TODO: manage the nullptr case.
		check(SkeletonMesh->GetSkeleton() == Skeleton);

		SkeletonMesh->Modify();

		SkeletonMesh->ReleaseResources();
		SkeletonMesh->ReleaseResourcesFence.Wait();

		// Add the root bone to the skeletal mesh's reference skeleton.
		AddRootBone(SkeletonMesh->GetSkeleton(), &SkeletonMesh->GetRefSkeleton());
		// Fix-up bone transforms and reset RetargetBasePose.
		SkeletonMesh->GetRetargetBasePose().Empty();
		SkeletonMesh->CalculateInvRefMatrices();	// @BUG: UE4 Undo system fails to undo the CalculateInvRefMatrices() effect.

		// As we added a new parent bone, fix "old" Skeletal Mesh indices.
		uint32 LODIndex = 0;
		for (FSkeletalMeshLODModel & LODModel : SkeletonMesh->GetImportedModel()->LODModels)
		{
			// == Fix the list of bones used by LODModel.

			// Increase old ActiveBoneIndices by 1, to compensate the new root bone.
			for (FBoneIndexType & i : LODModel.ActiveBoneIndices)
			{
				++i;
			}
			// Add the new root bone to the ActiveBoneIndices.
			LODModel.ActiveBoneIndices.Insert(0, 0);

			// Increase old RequiredBones by 1, to compensate the new root bone.
			for (FBoneIndexType & i : LODModel.RequiredBones)
			{
				++i;
			}
			// Add the new root bone to the RequiredBones.
			LODModel.RequiredBones.Insert(0, 0);

			// Updated the bone references used by the SkinWeightProfiles
			for (auto & Kvp : LODModel.SkinWeightProfiles)
			{
				FImportedSkinWeightProfileData & SkinWeightProfile = LODModel.SkinWeightProfiles.FindChecked(Kvp.Key);

				// Increase old InfluenceBones by 1, to compensate the new root bone.
				for (FRawSkinWeight & w : SkinWeightProfile.SkinWeights)
				{
					for (int i = 0; i < MAX_TOTAL_INFLUENCES; ++ i)
					{
						if (w.InfluenceWeights[i] > 0)
						{
							++ w.InfluenceBones[i];
						}
					}
				}

				// Increase old BoneIndex by 1, to compensate the new root bone.
				for (SkeletalMeshImportData::FVertInfluence & v: SkinWeightProfile.SourceModelInfluences)
				{
					if (v.Weight > 0)
					{
						++ v.BoneIndex;
					}
				}
			}

			// == Fix the mesh LOD sections.

			// Since UE4.24, newly imported Skeletal Mesh asset (UASSET) are serialized with additional data
			// and are processed differently. On the post-edit change of the asset, the editor automatically
			// re-builds all the sections starting from the stored raw mesh, if available.
			// This is made to properly re-apply the reduction settings after changes.
			// In this case, we must update the bones in the raw mesh and the editor will rebuild LODModel.Sections.
			if (SkeletonMesh->IsLODImportedDataBuildAvailable(LODIndex) && !SkeletonMesh->IsLODImportedDataEmpty(LODIndex))
			{
				FSkeletalMeshImportData RawMesh;
				SkeletonMesh->LoadLODImportedData(LODIndex, RawMesh);

				// Increase old ParentIndex by 1, to compensate the new root bone.
				int32 NumRootChildren = 0;
				for (SkeletalMeshImportData::FBone & b : RawMesh.RefBonesBinary)
				{
					if (b.ParentIndex == INDEX_NONE)
					{
						NumRootChildren += b.NumChildren;
					}
					++ b.ParentIndex;
				}
				// Add the new root bone to the RefBonesBinary.
				check(NumRootChildren > 0);
				const SkeletalMeshImportData::FJointPos NewRootPos = { FTransform3f::Identity, 1.f, 100.f, 100.f, 100.f };
				const SkeletalMeshImportData::FBone NewRoot = { RootBoneName.ToString(), 0, NumRootChildren, INDEX_NONE, NewRootPos };
				RawMesh.RefBonesBinary.Insert(NewRoot, 0);

				// Increase old BoneIndex by 1, to compensate the new root bone.
				// Influences stores the pairs (vertex, bone), no need to add new items.
				for (SkeletalMeshImportData::FRawBoneInfluence & b : RawMesh.Influences)
				{
					++ b.BoneIndex;
				}

				if (RawMesh.MorphTargets.Num() > 0)
				{
					FMessageLog("LogMixamoToolkit").Warning(FText::FromString(TEXT("MorphTargets are not supported.")));
				}

				if (RawMesh.AlternateInfluences.Num() > 0)
				{
					FMessageLog("LogMixamoToolkit").Warning(FText::FromString(TEXT("AlternateInfluences are not supported.")));
				}

				SkeletonMesh->SaveLODImportedData(LODIndex, RawMesh);
			}
			else
			{
				// For Skeletal Mesh assets (UASSET) using a pre-UE4.24 format (or missing the raw mesh data),
				// we must manually update the LODModel.Sections to keep them synchronized with the new added root bone.
				for (FSkelMeshSection & LODSection : LODModel.Sections)
				{
					// Increase old BoneMap indices by 1, to compensate the new root bone.
					for (FBoneIndexType & i : LODSection.BoneMap)
					{
						++i;
					}
					// No need to add the new root bone to BoneMap, as no vertices would use it.
					//
					// No need to update LODSection.SoftVertices[] items as FSoftSkinVertex::InfluenceBones
					// contains indices over LODSection.BoneMap, that does't changed items positions.
				}
			}

            ++LODIndex;
		}

		SkeletonMesh->PostEditChange();
		SkeletonMesh->InitResources();

		// Use the modified skeletal mesh to recreate the Skeleton bones structure, so it'll contains also the new root bone.
		// NOTE: this would invalidate the animations.
		Skeleton->Modify();
		if (iMesh == 0)
		{
			// Use the first mesh to re-create the base bone tree...
			Skeleton->RecreateBoneTree(SkeletonMesh);
		}
		else
		{
			// ...and then merge into Skeleton any new bone from SkeletonMesh.
			Skeleton->MergeAllBonesToBoneTree(SkeletonMesh);
		}
	}
}



/**
Add the "root" bone to a Skeletal Mesh's Reference Skeleton (RefSkeleton).
RefSkeleton must be based on Skeleton.
*/
void FMixamoSkeletonRetargeter::AddRootBone(const USkeleton * Skeleton, FReferenceSkeleton * RefSkeleton) const
{
	check(Skeleton != nullptr);
	check(RefSkeleton != nullptr);
	checkf(RefSkeleton->FindBoneIndex(RootBoneName) == INDEX_NONE, TEXT("The reference skeleton has already a \"root\" bone."));

	//=== Create a new FReferenceSkeleton with the root bone added.
	FReferenceSkeleton NewRefSkeleton;
	{
		// Destructor rebuilds the ref-skeleton.
		FReferenceSkeletonModifier RefSkeletonModifier(NewRefSkeleton, Skeleton);

		// Add the new root bone.
		const FMeshBoneInfo Root(RootBoneName, RootBoneName.ToString(), INDEX_NONE);
		RefSkeletonModifier.Add(Root, FTransform::Identity);

		// Copy and update existing bones indexes to get rid of the added root bone.
		for (int32 i = 0; i < RefSkeleton->GetRawBoneNum(); ++i)
		{
			FMeshBoneInfo info = RefSkeleton->GetRawRefBoneInfo()[i];
			info.ParentIndex += 1;
			const FTransform & pose = RefSkeleton->GetRawRefBonePose()[i];
			RefSkeletonModifier.Add(info, pose);
		}
	}

	// Set the new Reference Skeleton.
	*RefSkeleton = NewRefSkeleton;
}



/**
Setup the "Translation Retargeting" options for Skeleton (that is expected to be a Mixamo skeleton).

This options are used by Unreal Engine to retarget animations using Skeleton
(and NOT to retarget animations using a different skeleton asset, this is done considering the retargeting pose instead).
The reason is that skeletal meshes using the same skeleton can have different sizes and proportions,
these options allow Unreal Engine to adapt an animation authored for a specific skeletal mesh to
a skeletal mesh with different proportions (but based on the same skeleton).

See:
- https://docs.unrealengine.com/latest/INT/Engine/Animation/AnimationRetargeting/index.html#settingupretargeting
- https://docs.unrealengine.com/latest/INT/Engine/Animation/RetargetingDifferentSkeletons/#retargetingadjustments
- https://docs.unrealengine.com/latest/INT/Engine/Animation/AnimHowTo/Retargeting/index.html#retargetingusingthesameskeleton
*/
void FMixamoSkeletonRetargeter::SetupTranslationRetargetingModes(USkeleton* Skeleton) const
{
	check(Skeleton != nullptr);

	const FReferenceSkeleton & RefSkeleton = Skeleton->GetReferenceSkeleton();
	Skeleton->Modify();

	// Convert all bones, starting from the root one, to "Skeleton".
	// This will ensure that all bones use the skeleton's static translation.
	const int32 RootIndex = 0;
#ifndef MAR_ADDROOTBONE_DISABLE_
	checkf(RefSkeleton.FindBoneIndex(RootBoneName) == RootIndex, TEXT("Root bone at index 0"));
#endif
	Skeleton->SetBoneTranslationRetargetingMode(RootIndex, EBoneTranslationRetargetingMode::Skeleton, true);
	// Set the Pelvis bone (in Mixamo it's called "Hips") to AnimationScaled.
	// This will make sure that the bone sits at the right height and is still animated.
	const int32 PelvisIndex = RefSkeleton.FindBoneIndex(TEXT("Hips"));
	if (PelvisIndex != INDEX_NONE)
	{
		Skeleton->SetBoneTranslationRetargetingMode(PelvisIndex, EBoneTranslationRetargetingMode::AnimationScaled);
	}
	// Find the Root bone, any IK bones, any Weapon bones you may be using or other marker-style bones and set them to Animation.
	// This will make sure that bone's translation comes from the animation data itself and is unchanged.
	// @TODO: do it for IK bones.
	Skeleton->SetBoneTranslationRetargetingMode(RootIndex, EBoneTranslationRetargetingMode::Animation);
}



/**
Configure the "retarget pose" of SkeletalMeshes to match the "reference pose" of ReferenceSkeleton.

This is the pose needed by Unreal Engine to properly retarget animations involving different skeletons.
Animations are handled as additive bone transformations respect to the base pose of the skeletal mesh
for which they have been authored.

The new "retarget base pose" is then stored/applied accordingly to the inputs.

@param SkeletalMeshes	Skeletal Meshes for which a "retarget pose" must be configured.
@param ReferenceSkeleton	The skeleton with the "reference pose" that must be matched.
	Here we use the term "reference" to indicate the skeleton we want to match,
	do not confuse it with the "reference skeleton" term used by Unreal Engine to indicate the actual
	and concrete skeleton used by a Skeletal Mesh or Skeleton asset.
@param PreserveCSBonesNames	The bone names, in SkeletalMeshes's reference skeletons, that must preserve their Component Space transform.
@param ParentChildBoneNamesToBypassOneChildConstraint A set of parent-child bone names (in SkeletalMeshes's reference skeletons) that must be forcefully oriented regardless of
			the children number of the parent bone.
@param EditToReference_BoneNamesMapping Mapping of bone names from the edited skeleton (ie. from SkeletalMeshes's reference skeletons) to
	ReferenceSkeleton
@param bApplyPoseToRetargetBasePose If true, the computed "retarget pose" is applied to the "Retarget Base Pose" of the processed Skeletal Mesh.
@param Controller The IK Retargeter Controller to use to store the computed "retarget poses" in the processed Skeletal Mesh. Can be nullptr.
*/
void FMixamoSkeletonRetargeter::RetargetBasePose(
	TArray<USkeletalMesh *> SkeletalMeshes,
	const USkeleton * ReferenceSkeleton,
	const TArray<FName>& PreserveCSBonesNames,
	const FStaticNamesMapper & EditToReference_BoneNamesMapping,
	const TArray<TPair<FName, FName>>& ParentChildBoneNamesToBypassOneChildConstraint,
	bool bApplyPoseToRetargetBasePose,
	UIKRetargeterController* Controller
) const
{
	check(ReferenceSkeleton != nullptr);
	check(Controller);
	checkf(bApplyPoseToRetargetBasePose || Controller != nullptr, TEXT("Computed retarget pose must be saved/applied somewhere"));

	// @NOTE: UE4 mannequin skeleton must have same pose & proportions as of its skeletal mesh.
	FSkeletonPoser poser(ReferenceSkeleton, ReferenceSkeleton->GetReferenceSkeleton().GetRefBonePose());

	// Retarget all Skeletal Meshes using Skeleton.
	for (USkeletalMesh * Mesh : SkeletalMeshes)
	{
		Controller->AddRetargetPose(Mesh->GetFName());

		// Some of Mixamo's bones need a different rotation respect to UE4 mannequin reference pose.
		// An analytics solution would be preferred, but (for now) preserving the CS pose of their
		// children bones works quite well.
		TArray<FTransform> MeshBonePose;
		poser.PoseBasedOnMappedBoneNames(Mesh, 
			PreserveCSBonesNames, 
			EditToReference_BoneNamesMapping, 
			ParentChildBoneNamesToBypassOneChildConstraint,
			MeshBonePose);

		if (bApplyPoseToRetargetBasePose)
		{
			FSkeletonPoser::ApplyPoseToRetargetBasePose(Mesh, MeshBonePose);
		}
		if (Controller != nullptr)
		{
			FSkeletonPoser::ApplyPoseToIKRetargetPose(Mesh, Controller, MeshBonePose);
		}

		//@todo Add IK bones if possible.
	}

	// Ensure the Controller is set with the pose of the rendered preview mesh.
	if (Controller != nullptr)
	{
		if (USkeletalMesh* PreviewMesh = Controller->GetTargetPreviewMesh())
		{
			Controller->SetCurrentRetargetPose(PreviewMesh->GetFName());
		}
		else
		{
			Controller->SetCurrentRetargetPose(Controller->GetAsset()->GetDefaultPoseName());
		}
	}
}



/**
Ask to the user the Skeleton to use as "reference" for the retargeting.

I.e. the one to which we want to retarget the currently processed skeleton.
*/
USkeleton * FMixamoSkeletonRetargeter::AskUserForTargetSkeleton() const
{
	TSharedRef<SWindow> WidgetWindow = SNew(SWindow)
		.Title(LOCTEXT("FMixamoSkeletonRetargeter_AskUserForTargetSkeleton_WindowTitle", "Select retargeting skeleton"))
		.ClientSize(FVector2D(500, 600))
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		.HasCloseButton(false);

	TSharedRef<SRiggedSkeletonPicker> RiggedSkeletonPicker = SNew(SRiggedSkeletonPicker)
		.Title(LOCTEXT("FMixamoSkeletonRetargeter_AskUserForTargetSkeleton_Title", "Select a Skeleton asset to use as retarget source."))
		.Description(LOCTEXT("FMixamoSkeletonRetargeter_AskUserForTargetSkeleton_Description", "For optimal results, it should be the standard Unreal Engine mannequin skeleton."))
		.OnShouldFilterAsset(SRiggedSkeletonPicker::FOnShouldFilterAsset::CreateRaw(this, &FMixamoSkeletonRetargeter::OnShouldFilterNonUEMannequinSkeletonAsset));

	WidgetWindow->SetContent(RiggedSkeletonPicker);
	GEditor->EditorAddModalWindow(WidgetWindow);

	return RiggedSkeletonPicker->GetSelectedSkeleton();
}


bool FMixamoSkeletonRetargeter::AskUserOverridingAssetsConfirmation(const TArray<UObject*>& AssetsToOverwrite) const
{
	TSharedRef<SWindow> WidgetWindow = SNew(SWindow)
		.Title(LOCTEXT("FMixamoSkeletonRetargeter_AskUserOverridingAssetsConfirmation_WindowTitle", "Overwrite confirmation"))
		.ClientSize(FVector2D(400, 450))
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		.HasCloseButton(false);

	TSharedRef<SOverridingAssetsConfirmationDialog> ConfirmationDialog 
		= SNew(SOverridingAssetsConfirmationDialog)
		.AssetsToOverwrite(AssetsToOverwrite);

	WidgetWindow->SetContent(ConfirmationDialog);
	GEditor->EditorAddModalWindow(WidgetWindow);

	return ConfirmationDialog->HasConfirmed();
}


/// Return the FAssetData of all the skeletal meshes based on Skeleton.
void FMixamoSkeletonRetargeter::GetAllSkeletalMeshesUsingSkeleton(const USkeleton * Skeleton, TArray<FAssetData> & SkeletalMeshes) const
{
	SkeletalMeshes.Empty();

	FARFilter Filter;
	Filter.ClassNames.Add(USkeletalMesh::StaticClass()->GetFName());
	Filter.bRecursiveClasses = true;
	const FString SkeletonString = FAssetData(Skeleton).GetExportTextName();
	Filter.TagsAndValues.Add(USkeletalMesh::GetSkeletonMemberName(), SkeletonString);
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().GetAssets(Filter, SkeletalMeshes);
}



/**
Return the Skeletal Mesh assets of all the skeletal meshes based on Skeleton.

This will load all the returned Skeletal Meshes.
*/
void FMixamoSkeletonRetargeter::GetAllSkeletalMeshesUsingSkeleton(const USkeleton* Skeleton, TArray<USkeletalMesh *>& SkeletalMeshes) const
{
	TArray<FAssetData> Assets;
	GetAllSkeletalMeshesUsingSkeleton(Skeleton, Assets);
	SkeletalMeshes.Reset(Assets.Num());
	for (FAssetData& Asset : Assets)
	{
		// This will load the asset if needed.
		SkeletalMeshes.Emplace(CastChecked<USkeletalMesh>(Asset.GetAsset()));
	}
}



/// If Skeleton doesn't already have a Preview Mesh, then set it to PreviewMesh.
void FMixamoSkeletonRetargeter::SetPreviewMesh(USkeleton * Skeleton, USkeletalMesh * PreviewMesh) const
{
	check(Skeleton != nullptr);

	if (Skeleton->GetPreviewMesh() == nullptr && PreviewMesh != nullptr)
		Skeleton->SetPreviewMesh(PreviewMesh);
}



void FMixamoSkeletonRetargeter::EnumerateAssetsToOverwrite(const USkeleton* Skeleton, const USkeleton* ReferenceSkeleton, TArray<UObject*>& AssetsToOverride) const
{
	auto AddIfExists = [&AssetsToOverride](const FString& PackagePath, const FString& AssetName)
	{
		const FString LongPackageName = PackagePath / AssetName;

		UObject* Package = StaticFindObject(UObject::StaticClass(), nullptr, *LongPackageName);

		if (!Package)
		{
			Package = LoadPackage(nullptr, *LongPackageName, LOAD_NoWarn);
		}

		if (Package)
		{
			if (UObject* Obj = FindObject<UObject>(Package, *AssetName))
			{
				AssetsToOverride.AddUnique(Obj);
			}
		}
	};

	{
		const FString PackagePath = FPackageName::GetLongPackagePath(Skeleton->GetPackage()->GetName());
		AddIfExists(PackagePath, GetRigName(Skeleton));
	}
	{
		const FString PackagePath = FPackageName::GetLongPackagePath(ReferenceSkeleton->GetPackage()->GetName());
		AddIfExists(PackagePath, GetRigName(ReferenceSkeleton));
	}
	{
		const FString PackagePath = FPackageName::GetLongPackagePath(Skeleton->GetPackage()->GetName());
		AddIfExists(PackagePath, GetRetargeterName(Skeleton, ReferenceSkeleton));
	}
	{
		const FString PackagePath = FPackageName::GetLongPackagePath(Skeleton->GetPackage()->GetName());
		AddIfExists(PackagePath, GetRetargeterName(ReferenceSkeleton, Skeleton));
	}
}



/**
Create an IK Rig asset for Skeleton (it's Preview Mesh).
*/
UIKRigDefinition* FMixamoSkeletonRetargeter::CreateIKRig(
	const FString & PackagePath,
	const FString & AssetName,
	const USkeleton* Skeleton
) const
{
	check(Skeleton);

	const FString LongPackageName = PackagePath / AssetName;
	UPackage* Package = UPackageTools::FindOrCreatePackageForAssetType(FName(*LongPackageName), UIKRigDefinition::StaticClass());
	check(Package);

	UIKRigDefinition* IKRig = NewObject<UIKRigDefinition>(Package, FName(*AssetName), RF_Standalone | RF_Public | RF_Transactional);

	// Notify the asset registry
	FAssetRegistryModule::AssetCreated(IKRig);
	// Mark the package dirty...
	Package->MarkPackageDirty();

	// imports the skeleton data into the IK Rig
	UIKRigController* Controller = UIKRigController::GetIKRigController(IKRig);
	USkeletalMesh* SM = Skeleton->GetPreviewMesh();
	check(SM != nullptr);
	Controller->SetSkeletalMesh(SM);

	return IKRig;
}



/**
Create an IK Rig asset for a Mixamo Skeleton (it's Preview Mesh).
*/
UIKRigDefinition* FMixamoSkeletonRetargeter::CreateMixamoIKRig(const USkeleton* Skeleton) const
{
	check(Skeleton);

	const FString PackagePath = FPackageName::GetLongPackagePath(Skeleton->GetPackage()->GetName());
	UIKRigDefinition* IKRig = CreateIKRig(PackagePath, GetRigName(Skeleton), Skeleton);

	// imports the skeleton data into the IK Rig
	UIKRigController* Controller = UIKRigController::GetIKRigController(IKRig);
	USkeletalMesh* SM = Skeleton->GetPreviewMesh();
	check(SM != nullptr);
	Controller->SetSkeletalMesh(SM);

	static const FName RetargetRootBone(TEXT("Hips"));

#ifndef MAR_IKRETARGETER_ADVANCED_CHAINS_DISABLE_

	Controller->AddRetargetChain(TEXT("Root"), TEXT("root"), TEXT("root"));
	Controller->AddRetargetChain(TEXT("Spine"), TEXT("Spine"), TEXT("Spine2"));
	Controller->AddRetargetChain(TEXT("Head"), TEXT("Neck"), TEXT("head"));
	static const FName LeftClavicleChainName(TEXT("LeftClavicle"));
	Controller->AddRetargetChain(LeftClavicleChainName, TEXT("LeftShoulder"), TEXT("LeftShoulder"));
	static const FName LeftArmChainName(TEXT("LeftArm"));
	static const FName LeftHandBoneName(TEXT("LeftHand"));
	Controller->AddRetargetChain(LeftArmChainName, TEXT("LeftArm"), LeftHandBoneName);
	static const FName RightClavicleChainName(TEXT("RightClavicle"));
	Controller->AddRetargetChain(RightClavicleChainName, TEXT("RightShoulder"), TEXT("RightShoulder"));
	static const FName RightArmChainName(TEXT("RightArm"));
	static const FName RightHandBoneName(TEXT("RightHand"));
	Controller->AddRetargetChain(RightArmChainName, TEXT("RightArm"), RightHandBoneName);
	static const FName LeftLegChainName(TEXT("LeftLeg"));
	static const FName LeftToeBaseBoneName(TEXT("LeftToeBase"));
	Controller->AddRetargetChain(LeftLegChainName, TEXT("LeftUpLeg"), LeftToeBaseBoneName);
	static const FName RightLegChainName(TEXT("RightLeg"));
	static const FName RightToeBaseBoneName(TEXT("RightToeBase"));
	Controller->AddRetargetChain(RightLegChainName, TEXT("RightUpLeg"), RightToeBaseBoneName);
	Controller->AddRetargetChain(TEXT("LeftIndex"), TEXT("LeftHandIndex1"), TEXT("LeftHandIndex3"));
	Controller->AddRetargetChain(TEXT("RightIndex"), TEXT("RightHandIndex1"), TEXT("RightHandIndex3"));
	Controller->AddRetargetChain(TEXT("LeftMiddle"), TEXT("LeftHandMiddle1"), TEXT("LeftHandMiddle3"));
	Controller->AddRetargetChain(TEXT("RightMiddle"), TEXT("RightHandMiddle1"), TEXT("RightHandMiddle3"));
	Controller->AddRetargetChain(TEXT("LeftPinky"), TEXT("LeftHandPinky1"), TEXT("LeftHandPinky3"));
	Controller->AddRetargetChain(TEXT("RightPinky"), TEXT("RightHandPinky1"), TEXT("RightHandPinky3"));
	Controller->AddRetargetChain(TEXT("LeftRing"), TEXT("LeftHandRing1"), TEXT("LeftHandRing3"));
	Controller->AddRetargetChain(TEXT("RightRing"), TEXT("RightHandRing1"), TEXT("RightHandRing3"));
	Controller->AddRetargetChain(TEXT("LeftThumb"), TEXT("LeftHandThumb1"), TEXT("LeftHandThumb3"));
	Controller->AddRetargetChain(TEXT("RightThumb"), TEXT("RightHandThumb1"), TEXT("RightHandThumb3"));

#ifndef MAR_IKRETARGETER_IKSOLVERS_DISABLE_

	const int32 SolverIndex = Controller->AddSolver(UIKRigPBIKSolver::StaticClass());
	UIKRigPBIKSolver* Solver = CastChecked<UIKRigPBIKSolver>(Controller->GetSolver(SolverIndex));
	Solver->SetRootBone(RetargetRootBone);
	//Solver->RootBehavior = EPBIKRootBehavior::PinToInput;

	// Hips bone settings
	static const FName HipsBoneName(TEXT("Hips"));
	Controller->AddBoneSetting(HipsBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* HipsBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(HipsBoneName, SolverIndex)))
	{
		HipsBoneSettings->RotationStiffness = 0.99f;
	}

	// Spine bone settings
	static const FName SpineBoneName(TEXT("Spine"));
	Controller->AddBoneSetting(SpineBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* SpineBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(SpineBoneName, SolverIndex)))
	{
		SpineBoneSettings->RotationStiffness = 0.7f;
	}

	// Spine1 bone settings
	static const FName Spine1BoneName(TEXT("Spine1"));
	Controller->AddBoneSetting(Spine1BoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* Spine1BoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(Spine1BoneName, SolverIndex)))
	{
		Spine1BoneSettings->RotationStiffness = 0.8f;
	}

	// Spine2 bone settings
	static const FName Spine2BoneName(TEXT("Spine2"));
	Controller->AddBoneSetting(Spine2BoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* Spine2BoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(Spine2BoneName, SolverIndex)))
	{
		Spine2BoneSettings->RotationStiffness = 0.95f;
	}

	// Left Shoulder bone settings
	static const FName LeftShoulderBoneName(TEXT("LeftShoulder"));
	Controller->AddBoneSetting(LeftShoulderBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* LeftShoulderBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(LeftShoulderBoneName, SolverIndex)))
	{
		LeftShoulderBoneSettings->RotationStiffness = 0.99f;
	}

	// Left Hand goal
	static const FName LeftHandGoalName(TEXT("LeftHand_Goal"));
	if (UIKRigEffectorGoal* LeftHandGoal = Controller->AddNewGoal(LeftHandGoalName, LeftHandBoneName))
	{
		LeftHandGoal->bExposePosition = true;
		LeftHandGoal->bExposeRotation = true;
		Controller->ConnectGoalToSolver(*LeftHandGoal, SolverIndex);
		Controller->SetRetargetChainGoal(LeftArmChainName, LeftHandGoalName);
		if (UIKRig_FBIKEffector* LeftHandGoalSettings = Cast<UIKRig_FBIKEffector>(Controller->GetGoalSettingsForSolver(LeftHandGoalName, SolverIndex)))
		{
			LeftHandGoalSettings->PullChainAlpha = 0.f;
		}
	}

	// Right Shoulder bone settings
	static const FName RightShoulderBoneName(TEXT("RightShoulder"));
	Controller->AddBoneSetting(RightShoulderBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* RightShoulderBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(RightShoulderBoneName, SolverIndex)))
	{
		RightShoulderBoneSettings->RotationStiffness = 0.99f;
	}

	// Right Hand goal
	static const FName RightHandGoalName(TEXT("RightHand_Goal"));
	if (UIKRigEffectorGoal* RightHandGoal = Controller->AddNewGoal(RightHandGoalName, RightHandBoneName))
	{
		RightHandGoal->bExposePosition = true;
		RightHandGoal->bExposeRotation = true;
		Controller->ConnectGoalToSolver(*RightHandGoal, SolverIndex);
		Controller->SetRetargetChainGoal(RightArmChainName, RightHandGoalName);
		if (UIKRig_FBIKEffector* RightHandGoalSettings = Cast<UIKRig_FBIKEffector>(Controller->GetGoalSettingsForSolver(RightHandGoalName, SolverIndex)))
		{
			RightHandGoalSettings->PullChainAlpha = 0.f;
		}
	}

	// Left forearm settings
	static const FName LeftForeArmBoneName(TEXT("LeftForeArm"));
	Controller->AddBoneSetting(LeftForeArmBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* LeftForeArmBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(LeftForeArmBoneName, SolverIndex)))
	{
		ConfigureBonePreferredAnglesLocalToBS(
			Skeleton,
			LeftForeArmBoneSettings,
			FName("LeftHand"),
			{ 0, -90, 0 },
			FVector::UpVector
		);
	}

	// Right forearm settings
	static const FName RightForeArmBoneName(TEXT("RightForeArm"));
	Controller->AddBoneSetting(RightForeArmBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* RightForeArmBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(RightForeArmBoneName, SolverIndex)))
	{
		ConfigureBonePreferredAnglesLocalToBS(
			Skeleton,
			RightForeArmBoneSettings,
			FName("RightHand"),
			{ 0, 90, 0 },
			FVector::UpVector
		);
	}

	// Left Up Leg bone settings
	static const FName LeftUpLegBoneName(TEXT("LeftUpLeg"));
	Controller->AddBoneSetting(LeftUpLegBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* LeftUpLegBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(LeftUpLegBoneName, SolverIndex)))
	{
		ConfigureBonePreferredAnglesLocalToBS(
			Skeleton,
			LeftUpLegBoneSettings,
			FName("LeftLeg"),
			{ 0, -90, 0 },
			FVector::ForwardVector
		);
	}

	// Left Leg bone settings
	static const FName LeftLegBoneName(TEXT("LeftLeg"));
	Controller->AddBoneSetting(LeftLegBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* LeftLegBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(LeftLegBoneName, SolverIndex)))
	{
		ConfigureBonePreferredAnglesLocalToBS(
			Skeleton,
			LeftLegBoneSettings,
			FName("LeftFoot"),
			{ 0, 90, 0 },
			FVector::ForwardVector
		);
	}

	// Left Foot goal
	static const FName LeftFootGoalName(TEXT("LeftFoot_Goal"));
	if (UIKRigEffectorGoal* LeftFootGoal = Controller->AddNewGoal(LeftFootGoalName, LeftToeBaseBoneName))
	{
		LeftFootGoal->bExposePosition = true;
		LeftFootGoal->bExposeRotation = true;
		Controller->ConnectGoalToSolver(*LeftFootGoal, SolverIndex);
		Controller->SetRetargetChainGoal(LeftLegChainName, LeftFootGoalName);
	}

	// Right Up Leg bone settings
	static const FName RightUpLegBoneName(TEXT("RightUpLeg"));
	Controller->AddBoneSetting(RightUpLegBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* RightUpLegBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(RightUpLegBoneName, SolverIndex)))
	{
		ConfigureBonePreferredAnglesLocalToBS(
			Skeleton,
			RightUpLegBoneSettings,
			FName("RightLeg"),
			{ 0, -90, 0 },
			FVector::ForwardVector
		);
	}

	// Right Leg bone settings
	static const FName RightLegBoneName(TEXT("RightLeg"));
	Controller->AddBoneSetting(RightLegBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* RightLegBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(RightLegBoneName, SolverIndex)))
	{
		ConfigureBonePreferredAnglesLocalToBS(
			Skeleton,
			RightLegBoneSettings,
			FName("RightFoot"),
			{ 0, 90, 0 },
			FVector::ForwardVector
		);
	}

	// Right Foot goal
	static const FName RightFootGoalName(TEXT("RightFoot_Goal"));
	if (UIKRigEffectorGoal* RightFootGoal = Controller->AddNewGoal(RightFootGoalName, RightToeBaseBoneName))
	{
		RightFootGoal->bExposePosition = true;
		RightFootGoal->bExposeRotation = true;
		Controller->ConnectGoalToSolver(*RightFootGoal, SolverIndex);
		Controller->SetRetargetChainGoal(RightLegChainName, RightFootGoalName);
	}

#endif // MAR_IKRETARGETER_IKSOLVERS_DISABLE_
	
#else // MAR_IKRETARGETER_ADVANCED_CHAINS_DISABLE_

	TArray<FName> BoneNames;
	UE4MannequinToMixamo_BoneNamesMapping.GetDestination(BoneNames);	// NOTE: it's ok as long as the UE5 version has the same destination bone names.
	for (const FName& BoneName : BoneNames)
	{
		Controller->AddRetargetChain(/*ChainName=*/BoneName, /*StartBoneName=*/BoneName, /*EndBoneName=*/BoneName);
	}

#endif // MAR_IKRETARGETER_ADVANCED_CHAINS_DISABLE_

	Controller->SetRetargetRoot(RetargetRootBone);

	return IKRig;
}



/**
Create an IK Rig asset for a UE Mannequin Skeleton (it's Preview Mesh).
*/
UIKRigDefinition* FMixamoSkeletonRetargeter::CreateUEMannequinIKRig(
	const USkeleton* Skeleton,
	ETargetSkeletonType SkeletonType
) const
{
	check(Skeleton != nullptr);

	const FString PackagePath = FPackageName::GetLongPackagePath(Skeleton->GetPackage()->GetName());
	UIKRigDefinition* IKRig = CreateIKRig(PackagePath, GetRigName(Skeleton), Skeleton);

	// imports the skeleton data into the IK Rig
	UIKRigController* Controller = UIKRigController::GetIKRigController(IKRig);
	USkeletalMesh* SM = Skeleton->GetPreviewMesh();
	check(SM != nullptr);
	Controller->SetSkeletalMesh(SM);

	static const FName RetargetRootBone(TEXT("pelvis"));

	const bool bIsUE5Skeleton = SkeletonType == ETargetSkeletonType::ST_UE5_MANNEQUIN;

#ifndef MAR_IKRETARGETER_ADVANCED_CHAINS_DISABLE_

	Controller->AddRetargetChain(TEXT("Root"), TEXT("root"), TEXT("root"));
	Controller->AddRetargetChain(TEXT("Spine"), TEXT("spine_01"), SelectBySkeletonType(SkeletonType, TEXT("spine_03"), TEXT("spine_05")));
	Controller->AddRetargetChain(TEXT("Head"), TEXT("neck_01"), TEXT("head"));
	static const FName LeftClavicleChainName(TEXT("LeftClavicle"));
	Controller->AddRetargetChain(LeftClavicleChainName, TEXT("clavicle_l"), TEXT("clavicle_l"));
	static const FName LeftArmChainName(TEXT("LeftArm"));
	static const FName LeftHandBoneName(TEXT("hand_l"));
	Controller->AddRetargetChain(LeftArmChainName, TEXT("upperarm_l"), LeftHandBoneName);
	static const FName RightClavicleChainName(TEXT("RightClavicle"));
	Controller->AddRetargetChain(RightClavicleChainName, TEXT("clavicle_r"), TEXT("clavicle_r"));
	static const FName RightArmChainName(TEXT("RightArm"));
	static const FName RightHandBoneName(TEXT("hand_r"));
	Controller->AddRetargetChain(RightArmChainName, TEXT("upperarm_r"), RightHandBoneName);
	static const FName LeftLegChainName(TEXT("LeftLeg"));
	static const FName LeftBallBoneName(TEXT("ball_l"));
	Controller->AddRetargetChain(TEXT("LeftLeg"), TEXT("thigh_l"), LeftBallBoneName);
	static const FName RightLegChainName(TEXT("RightLeg"));
	static const FName RightBallBoneName(TEXT("ball_r"));
	Controller->AddRetargetChain(RightLegChainName, TEXT("thigh_r"), RightBallBoneName);
	Controller->AddRetargetChain(TEXT("LeftIndex"), TEXT("index_01_l"), TEXT("index_03_l"));
	Controller->AddRetargetChain(TEXT("RightIndex"), TEXT("index_01_r"), TEXT("index_03_r"));
	Controller->AddRetargetChain(TEXT("LeftMiddle"), TEXT("middle_01_l"), TEXT("middle_03_l"));
	Controller->AddRetargetChain(TEXT("RightMiddle"), TEXT("middle_01_r"), TEXT("middle_03_r"));
	Controller->AddRetargetChain(TEXT("LeftPinky"), TEXT("pinky_01_l"), TEXT("pinky_03_l"));
	Controller->AddRetargetChain(TEXT("RightPinky"), TEXT("pinky_01_r"), TEXT("pinky_03_r"));
	Controller->AddRetargetChain(TEXT("LeftRing"), TEXT("ring_01_l"), TEXT("ring_03_l"));
	Controller->AddRetargetChain(TEXT("RightRing"), TEXT("ring_01_r"), TEXT("ring_03_r"));
	Controller->AddRetargetChain(TEXT("LeftThumb"), TEXT("thumb_01_l"), TEXT("thumb_03_l"));
	Controller->AddRetargetChain(TEXT("RightThumb"), TEXT("thumb_01_r"), TEXT("thumb_03_r"));
	if (bIsUE5Skeleton)
	{
		// If we don't add them (also if apparently useless), the IK Retargeter editor (UE5.0.2) wrongly processes the
		// descendant bones hierarchy (see Issue #863): the metacarpal bones are not drawn in the editor and
		// children bones have a wrong trasformation applied, resulting in a wrong pose of the fingers.
		Controller->AddRetargetChain(TEXT("LeftIndexMetacarpal"), TEXT("index_metacarpal_l"), TEXT("index_metacarpal_l"));
		Controller->AddRetargetChain(TEXT("RightIndexMetacarpal"), TEXT("index_metacarpal_r"), TEXT("index_metacarpal_r"));
		Controller->AddRetargetChain(TEXT("LeftMiddleMetacarpal"), TEXT("middle_metacarpal_l"), TEXT("middle_metacarpal_l"));
		Controller->AddRetargetChain(TEXT("RightMiddleMetacarpal"), TEXT("middle_metacarpal_r"), TEXT("middle_metacarpal_r"));
		Controller->AddRetargetChain(TEXT("LeftPinkyMetacarpal"), TEXT("pinky_metacarpal_l"), TEXT("pinky_metacarpal_l"));
		Controller->AddRetargetChain(TEXT("RightPinkyMetacarpal"), TEXT("pinky_metacarpal_r"), TEXT("pinky_metacarpal_r"));
		Controller->AddRetargetChain(TEXT("LeftRingMetacarpal"), TEXT("ring_metacarpal_l"), TEXT("ring_metacarpal_l"));
		Controller->AddRetargetChain(TEXT("RightRingMetacarpal"), TEXT("ring_metacarpal_r"), TEXT("ring_metacarpal_r"));
	}
	// NOTE: for ST_UE5_MANNEQUIN are missing: all *Twist*, *IK chains.

#ifndef MAR_IKRETARGETER_IKSOLVERS_DISABLE_

	const int32 SolverIndex = Controller->AddSolver(UIKRigPBIKSolver::StaticClass());
	UIKRigPBIKSolver* Solver = CastChecked<UIKRigPBIKSolver>(Controller->GetSolver(SolverIndex));
	Solver->SetRootBone(RetargetRootBone);
	//Solver->RootBehavior = EPBIKRootBehavior::PinToInput;

	// Pelvis bone settings
	static const FName PelvisBoneName(TEXT("pelvis"));
	Controller->AddBoneSetting(PelvisBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* HipsBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(PelvisBoneName, SolverIndex)))
	{
		HipsBoneSettings->RotationStiffness = 1.0f;
	}

	// Spine_01 bone settings
	static const FName Spine1BoneName(TEXT("spine_01"));
	Controller->AddBoneSetting(Spine1BoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* Spine1BoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(Spine1BoneName, SolverIndex)))
	{
		Spine1BoneSettings->RotationStiffness = SelectBySkeletonType(SkeletonType, 0.784f, 0.896f);
	}

	// Spine_02 bone settings
	static const FName Spine2BoneName(TEXT("spine_02"));
	Controller->AddBoneSetting(Spine2BoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* Spine2BoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(Spine2BoneName, SolverIndex)))
	{
		Spine2BoneSettings->RotationStiffness = SelectBySkeletonType(SkeletonType, 0.928f, 0.936f);
	}

	// Spine_03 bone settings
	static const FName Spine3BoneName(TEXT("spine_03"));
	Controller->AddBoneSetting(Spine3BoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* Spine3BoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(Spine3BoneName, SolverIndex)))
	{
		Spine3BoneSettings->RotationStiffness = 0.936f;
	}

	if (bIsUE5Skeleton)
	{
		// Spine_04 bone settings
		static const FName Spine4BoneName(TEXT("spine_04"));
		Controller->AddBoneSetting(Spine4BoneName, SolverIndex);
		if (UIKRig_PBIKBoneSettings* Spine4BoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(Spine4BoneName, SolverIndex)))
		{
			Spine4BoneSettings->RotationStiffness = 0.936f;
		}

		// Spine_05 bone settings
		static const FName Spine5BoneName(TEXT("spine_05"));
		Controller->AddBoneSetting(Spine5BoneName, SolverIndex);
		if (UIKRig_PBIKBoneSettings* Spine5BoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(Spine5BoneName, SolverIndex)))
		{
			Spine5BoneSettings->RotationStiffness = 0.936f;
		}
	}

	// Clavicle Left bone settings
	static const FName ClavicleLeftBoneName(TEXT("clavicle_l"));
	Controller->AddBoneSetting(ClavicleLeftBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* ClavicleLeftBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(ClavicleLeftBoneName, SolverIndex)))
	{
		ClavicleLeftBoneSettings->RotationStiffness = 1.0f;
	}

	// Left Lower arm bone settings
	static const FName LowerArmLeftBoneName(TEXT("lowerarm_l"));
	Controller->AddBoneSetting(LowerArmLeftBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* LowerArmLeftBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(LowerArmLeftBoneName, SolverIndex)))
	{
		LowerArmLeftBoneSettings->bUsePreferredAngles = true;
		LowerArmLeftBoneSettings->PreferredAngles = { 0, 0, 90 };
	}

	// Left Hand goal
	static const FName LeftHandGoalName(TEXT("hand_l_Goal"));
	if (UIKRigEffectorGoal* LeftHandGoal = Controller->AddNewGoal(LeftHandGoalName, LeftHandBoneName))
	{
		LeftHandGoal->bExposePosition = true;
		LeftHandGoal->bExposeRotation = true;
		Controller->ConnectGoalToSolver(*LeftHandGoal, SolverIndex);
		Controller->SetRetargetChainGoal(LeftArmChainName, LeftHandGoalName);
		if (UIKRig_FBIKEffector* LeftHandGoalSettings = Cast<UIKRig_FBIKEffector>(Controller->GetGoalSettingsForSolver(LeftHandGoalName, SolverIndex)))
		{
			LeftHandGoalSettings->PullChainAlpha = 0.f;
		}
	}

	// Clavicle Right bone settings
	static const FName ClavicleRightBoneName(TEXT("clavicle_r"));
	Controller->AddBoneSetting(ClavicleRightBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* ClavicleRightBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(ClavicleRightBoneName, SolverIndex)))
	{
		ClavicleRightBoneSettings->RotationStiffness = 1.0f;
	}

	// Right Lower arm bone settings
	static const FName LowerArmRightBoneName(TEXT("lowerarm_r"));
	Controller->AddBoneSetting(LowerArmRightBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* LowerArmRightBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(LowerArmRightBoneName, SolverIndex)))
	{
		LowerArmRightBoneSettings->bUsePreferredAngles = true;
		LowerArmRightBoneSettings->PreferredAngles = { 0, 0, 90 };
	}

	// Right Hand goal
	static const FName RightHandGoalName(TEXT("hand_r_Goal"));
	if (UIKRigEffectorGoal* RightHandGoal = Controller->AddNewGoal(RightHandGoalName, RightHandBoneName))
	{
		RightHandGoal->bExposePosition = true;
		RightHandGoal->bExposeRotation = true;
		Controller->ConnectGoalToSolver(*RightHandGoal, SolverIndex);
		Controller->SetRetargetChainGoal(RightArmChainName, RightHandGoalName);
		if (UIKRig_FBIKEffector* RightHandGoalSettings = Cast<UIKRig_FBIKEffector>(Controller->GetGoalSettingsForSolver(RightHandGoalName, SolverIndex)))
		{
			RightHandGoalSettings->PullChainAlpha = 0.f;
		}
	}

	// Left Leg bone settings
	static const FName LeftLegBoneName(TEXT("calf_l"));
	Controller->AddBoneSetting(LeftLegBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* LeftLegBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(LeftLegBoneName, SolverIndex)))
	{
		LeftLegBoneSettings->bUsePreferredAngles = true;
		LeftLegBoneSettings->PreferredAngles = { 0, 0, 90 };
	}
	
	// Left Foot goal
	static const FName LeftFootGoalName(TEXT("foot_l_Goal"));
	if (UIKRigEffectorGoal* LeftFootGoal = Controller->AddNewGoal(LeftFootGoalName, LeftBallBoneName))
	{
		LeftFootGoal->bExposePosition = true;
		LeftFootGoal->bExposeRotation = true;
		Controller->ConnectGoalToSolver(*LeftFootGoal, SolverIndex);
		Controller->SetRetargetChainGoal(LeftLegChainName, LeftFootGoalName);
	}

	// Right Leg bone settings
	static const FName RightLegBoneName(TEXT("calf_r"));
	Controller->AddBoneSetting(RightLegBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* RightLegBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(RightLegBoneName, SolverIndex)))
	{
		RightLegBoneSettings->bUsePreferredAngles = true;
		RightLegBoneSettings->PreferredAngles = { 0, 0, 90 };
	}

	// Right Foot goal
	static const FName RightFootGoalName(TEXT("foot_r_Goal"));
	if (UIKRigEffectorGoal* RightFootGoal = Controller->AddNewGoal(RightFootGoalName, RightBallBoneName))
	{
		RightFootGoal->bExposePosition = true;
		RightFootGoal->bExposeRotation = true;
		Controller->ConnectGoalToSolver(*RightFootGoal, SolverIndex);
		Controller->SetRetargetChainGoal(RightLegChainName, RightFootGoalName);
	}

	// Left Thigh bone settings
	static const FName LeftThighBoneName(TEXT("thigh_l"));
	Controller->AddBoneSetting(LeftThighBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* LeftThighBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(LeftThighBoneName, SolverIndex)))
	{
		LeftThighBoneSettings->bUsePreferredAngles = true;
		LeftThighBoneSettings->PreferredAngles = { 0, 0, -90 };
	}

	// Right Thigh bone settings
	static const FName RightThighBoneName(TEXT("thigh_r"));
	Controller->AddBoneSetting(RightThighBoneName, SolverIndex);
	if (UIKRig_PBIKBoneSettings* RightThighBoneSettings = Cast<UIKRig_PBIKBoneSettings>(Controller->GetSettingsForBone(RightThighBoneName, SolverIndex)))
	{
		RightThighBoneSettings->bUsePreferredAngles = true;
		RightThighBoneSettings->PreferredAngles = { 0, 0, -90 };
	}

#endif // MAR_IKRETARGETER_IKSOLVERS_DISABLE_

#else // MAR_IKRETARGETER_ADVANCED_CHAINS_DISABLE_

	TArray<FName> BoneNames;
	const FStaticNamesMapper& UEMannequinToMixamo_BoneNamesMapping = SelectBySkeletonType(SkeletonType, UE4MannequinToMixamo_BoneNamesMapping, UE5MannequinToMixamo_BoneNamesMapping);
	UEMannequinToMixamo_BoneNamesMapping.GetSource(BoneNames);
	for (const FName& BoneName : BoneNames)
	{
		Controller->AddRetargetChain(/*ChainName=*/BoneName, /*StartBoneName=*/BoneName, /*EndBoneName=*/BoneName);
	}

#endif // MAR_IKRETARGETER_ADVANCED_CHAINS_DISABLE_

	Controller->SetRetargetRoot(RetargetRootBone);

	return IKRig;
}



/**
Create an IK Retargeter asset from Source Rig to Target Rig.

@param TargetToSource_ChainNamesMapping Mapper of chain names from the TargetRig to the SourceRig.
@param BoneChainsToSkip Set of IK Rig chain names (relative to TargetRig) for which a "retarget chain" must not be configured.
@param TargetBoneChainsDriveIKGoalToSource Set of IK Rig chain names (relative to TargetRig) for which a the "Drive IK Goal" must be configured.
*/
UIKRetargeter* FMixamoSkeletonRetargeter::CreateIKRetargeter(
	const FString & PackagePath,
	const FString & AssetName,
	UIKRigDefinition* SourceRig,
	UIKRigDefinition* TargetRig,
	const FStaticNamesMapper & TargetToSource_ChainNamesMapping,
	const TArray<FName> & TargetBoneChainsToSkip,
	const TArray<FName> & TargetBoneChainsDriveIKGoal,
	const TArray<FName>& TargetBoneChainsOneToOneRotationMode
) const
{
	/*
	// Using the FAssetToolsModule (add AssetTools to Build.cs and #include "AssetToolsModule.h"):

	FAssetToolsModule& AssetToolsModule = FAssetToolsModule::GetModule();
	FString UniquePackageName;
	FString UniqueAssetName;
	AssetToolsModule.Get().CreateUniqueAssetName(PackagePath / AssetName, TEXT(""), UniquePackageName, UniqueAssetName);
	const FString UniquePackagePath = FPackageName::GetLongPackagePath(UniquePackageName);

	UIKRetargeter* Retargeter = CastChecked<UIKRetargeter>(AssetToolsModule.Get().CreateAsset(UniqueAssetName, UniquePackagePath, UIKRetargeter::StaticClass(), nullptr));
	*/

	const FString LongPackageName = PackagePath / AssetName;
	UPackage* Package = UPackageTools::FindOrCreatePackageForAssetType(FName(*LongPackageName), UIKRetargeter::StaticClass());
	check(Package);

	UIKRetargeter* Retargeter = NewObject<UIKRetargeter>(Package, FName(*AssetName), RF_Standalone | RF_Public | RF_Transactional);

	// Notify the asset registry
	FAssetRegistryModule::AssetCreated(Retargeter);
	// Mark the package dirty...
	Package->MarkPackageDirty();

	UIKRetargeterController* Controller = UIKRetargeterController::GetController(Retargeter);
	Controller->SetSourceIKRig(SourceRig);

	// Controller->SetTargetIKRig(TargetRig) is BUGGED, do not set the TargetIKRig! Set it with reflection.
	FObjectPropertyBase* TargetIKRigProperty = CastFieldChecked<FObjectPropertyBase>(
		UIKRetargeter::StaticClass()->FindPropertyByName(UIKRetargeter::GetTargetIKRigPropertyName()));
	if (ensure(TargetIKRigProperty))
	{
		void* ptr = TargetIKRigProperty->ContainerPtrToValuePtr<void>(Retargeter);
		check(ptr);
		TargetIKRigProperty->SetObjectPropertyValue(ptr, TargetRig);
	}

	Controller->CleanChainMapping();
	for (URetargetChainSettings* ChainMap : Controller->GetChainMappings())
	{
		// Check if we need to explicitly skip an existing bone chain.
		if (TargetBoneChainsToSkip.Contains(ChainMap->TargetChain))
		{
			continue;
		}
		// Search the mapped bone name.
		const FName SourceChainName = TargetToSource_ChainNamesMapping.MapName(ChainMap->TargetChain);
		// Skip if the targte chain name is not mapped.
		if (SourceChainName.IsNone())
		{
			continue;
		}

		// Add the Target->Source chain name association.
		Controller->SetSourceChainForTargetChain(ChainMap, SourceChainName);

		//= Configure the ChainMap settings

		// this is needed for root motion
		if (SourceChainName == FName("Root"))
		{
			ChainMap->TranslationMode = ERetargetTranslationMode::GloballyScaled;
		}

#ifndef MAR_IKRETARGETER_IKSOLVERS_DISABLE_
		// Configure the DriveIKGoal setting.
		ChainMap->DriveIKGoal = TargetBoneChainsDriveIKGoal.Contains(ChainMap->TargetChain);

		if (TargetBoneChainsOneToOneRotationMode.Contains(ChainMap->TargetChain))
		{
			ChainMap->RotationMode = ERetargetRotationMode::OneToOne;
		}
#endif
	}

	return Retargeter;
}


#undef LOCTEXT_NAMESPACE
