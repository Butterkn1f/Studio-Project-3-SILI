/**
 CSoundController
 @brief A class which manages the sound objects
 By: Toh Da Jun
 Date: Mar 2020
 */
#include "SoundController.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor
 */
CSoundController::CSoundController(void)
	: cSoundEngine(NULL)
	, vec3dfListenerPos(vec3df(0, 0, 0))
	, vec3dfListenerDir(vec3df(0, 0, 1))
{
}

/**
 @brief Destructor
 */
CSoundController::~CSoundController(void)
{
	// Iterate through the entityMap
	for (std::map<int, CSoundInfo*>::iterator it = soundMap.begin(); it != soundMap.end(); ++it)
	{
		// If the value/second was not deleted elsewhere, then delete it here
		if (it->second != NULL)
		{
			delete it->second;
			it->second = NULL;
		}
	}

	// Remove all elements in entityMap
	soundMap.clear();

	// Since we have already dropped the ISoundSource, then we don't need to delete the cSoundEngine
	//// Clear the sound engine
	//if (cSoundEngine)
	//{
	//	cSoundEngine->drop();
	//	cSoundEngine = NULL;
	//}
}

/**
 @brief Initialise this class instance
 @return A bool value. true is this class instance was initialised, else false
 */
bool CSoundController::Init(void)
{
	// Initialise the sound engine with default parameters
	cSoundEngine = createIrrKlangDevice(ESOD_WIN_MM, ESEO_MULTI_THREADED);
	if (cSoundEngine == NULL)
	{
		cout << "Unable to initialise the IrrKlang sound engine" << endl;
		return false;
	}
	return true;
}

/**
 @brief Load a sound
 @param filename A string variable storing the name of the file to read from
 @param ID A const int variable which will be the ID of the iSoundSource in the map
 @param bPreload A const bool variable which indicates if this iSoundSource will be pre-loaded into memory now.
 @param bIsLooped A const bool variable which indicates if this iSoundSource will have loop playback.
 @param eSoundType A SOUNDTYPE enum variable which states the type of sound
 @param vec3dfSoundPos A vec3df variable which contains the 3D position of the soundS
 */
bool CSoundController::LoadSound(string filename,
	const int ID,
	const bool bPreload,
	const bool bIsLooped,
	CSoundInfo::SOUNDTYPE eSoundType,
	vec3df vec3dfSoundPos)
{	
	// Load the sound from the file
	ISoundSource* pSoundSource = cSoundEngine->addSoundSourceFromFile(filename.c_str(),
		E_STREAM_MODE::ESM_NO_STREAMING,
		bPreload);

	// Trivial Rejection : Invalid pointer provided
	if (pSoundSource == nullptr)
	{
		//cout << "Unable to load sound " << filename.c_str() << endl;
		return false;
	}

	// Force the sound source not to have any streaming
	pSoundSource->setForcedStreamingThreshold(-1);

	// Clean up first if there is an existing Entity with the same name
	RemoveSound(ID);

	cout << vec3dfSoundPos.X << endl;

	// Add the entity now
	CSoundInfo* cSoundInfo = new CSoundInfo();
	if (eSoundType == CSoundInfo::SOUNDTYPE::_2D)
		cSoundInfo->Init(ID, pSoundSource, bIsLooped);
	else
		cSoundInfo->Init(ID, pSoundSource, bIsLooped, eSoundType, vec3dfSoundPos);

	// Set to soundMap
	soundMap[ID] = cSoundInfo;

	return true;
}

/**
 @brief Play a sound by its ID
 @param ID A const int variable which will be the ID of the iSoundSource in the map
 */
void CSoundController::PlaySoundByID(const int ID)
{
	CSoundInfo* pSoundInfo = GetSound(ID);
	if (!pSoundInfo)
	{
		cout << "Sound #" << ID << " is not playable." << endl;
		return;
	}
	else if (cSoundEngine->isCurrentlyPlaying(pSoundInfo->GetSound()))
	{
		//cout << "Sound #" << ID << " is currently being played." << endl;
		return;
	}

	if (pSoundInfo->GetSoundType() == CSoundInfo::SOUNDTYPE::_2D)
	{
		cSoundEngine->play2D(pSoundInfo->GetSound(),
			pSoundInfo->GetLoopStatus());
	}
	else if (pSoundInfo->GetSoundType() == CSoundInfo::SOUNDTYPE::_3D)
	{
		cSoundEngine->setListenerPosition(vec3dfListenerPos, vec3dfListenerDir);
		cSoundEngine->play3D(pSoundInfo->GetSound(),
			pSoundInfo->GetPosition(),
			pSoundInfo->GetLoopStatus());
	}
}

/**
 @brief Stop playing a sound by its ID
 @param ID A const int variable which will be the ID of the iSoundSource in the map
 */

void CSoundController::StopSoundByID(const int ID)
{
	CSoundInfo* pSoundInfo = GetSound(ID);
	if (!pSoundInfo)
	{
		cout << "Sound #" << ID << " is not playing now." << endl;
		return;
	}
	else if (cSoundEngine->isCurrentlyPlaying(pSoundInfo->GetSound()))
	{
		cout << "Sound #" << ID << " has been stopped." << endl;
		cSoundEngine->stopAllSoundsOfSoundSource(pSoundInfo->GetSound());
	}
}

/**
 @brief Increase Master volume
 @return true if successfully increased volume, else false
 */
bool CSoundController::MasterVolumeIncrease(void)
{
	// Get the current volume
	float fCurrentVolume = cSoundEngine->getSoundVolume() + 0.1f;
	// Check if the maximum volume has been reached
	if (fCurrentVolume > 1.0f)
		fCurrentVolume = 1.0f;

	// Update the Mastervolume
	cSoundEngine->setSoundVolume(fCurrentVolume);
	//cout << "MasterVolumeIncrease: fCurrentVolume = " << fCurrentVolume << endl;

	return true;
}

/**
 @brief Decrease Master volume
 @return true if successfully decreased volume, else false
 */
bool CSoundController::MasterVolumeDecrease(void)
{
	// Get the current volume
	float fCurrentVolume = cSoundEngine->getSoundVolume() - 0.1f;
	// Check if the minimum volume has been reached
	if (fCurrentVolume < 0.0f)
		fCurrentVolume = 0.0f;

	// Update the Mastervolume
	cSoundEngine->setSoundVolume(fCurrentVolume);
	cout << "MasterVolumeDecrease: fCurrentVolume = " << fCurrentVolume << endl;

	return true;
}


/**
 @brief Increase volume of a ISoundSource
 @param ID A const int variable which contains the ID of the iSoundSource in the map
 @return true if successfully decreased volume, else false
 */
bool CSoundController::VolumeIncrease(const int ID)
{
	// Get the ISoundSource
	ISoundSource* pISoundSource = GetSound(ID)->GetSound();
	if (pISoundSource == nullptr)
	{
		return false;
	}

	// Get the current volume
	float fCurrentVolume = pISoundSource->getDefaultVolume();

	// Check if the maximum volume has been reached
	if (fCurrentVolume >= 1.0f)
	{
		pISoundSource->setDefaultVolume(1.0f);
		return false;
	}

	// Increase the volume by 10%
	pISoundSource->setDefaultVolume(fCurrentVolume + 0.1f);

	return true;
}

/**
 @brief Decrease volume of a ISoundSource
 @param ID A const int variable which contains the ID of the iSoundSource in the map
 @return true if successfully decreased volume, else false
 */
bool CSoundController::VolumeDecrease(const int ID)
{
	// Get the ISoundSource
	ISoundSource* pISoundSource = GetSound(ID)->GetSound();
	if (pISoundSource == nullptr)
	{
		return false;
	}

	// Get the current volume
	float fCurrentVolume = pISoundSource->getDefaultVolume();

	// Check if the minimum volume has been reached
	if (fCurrentVolume <= 0.0f)
	{
		pISoundSource->setDefaultVolume(0.0f);
		return false;
	}

	// Decrease the volume by 10%
	pISoundSource->setDefaultVolume(fCurrentVolume - 0.1f);

	return true;
}

bool CSoundController::SetVolume(const int ID, const float volume)
{
	// Get the ISoundSource
	CSoundInfo* SoundSource = GetSound(ID);
	if (SoundSource == nullptr)
	{
		return false;
	}
	ISoundSource* pISoundSource = SoundSource->GetSound();
	if (pISoundSource == nullptr)
	{
		return false;
	}

	// Get the current volume
	float fCurrentVolume = pISoundSource->getDefaultVolume();

	//// Check if the minimum volume has been reached
	//if (fCurrentVolume <= 0.0f)
	//{
	//	pISoundSource->setDefaultVolume(0.0f);
	//	return false;
	//}

	pISoundSource->setDefaultVolume(volume);

	return true;
}

bool CSoundController::SetMasterVolume(const float volume)
{
	// Get the current volume
	float fCurrentVolume = cSoundEngine->getSoundVolume();
	// Check if the minimum volume has been reached
	if (fCurrentVolume < 0.0f)
		fCurrentVolume = 0.0f;

	// Update the Mastervolume
	cSoundEngine->setSoundVolume(volume);

	return true;
}

bool CSoundController::SetBGMVolume(const float volume)
{
	SetVolume(1, volume);
	SetVolume(3, volume);
	SetVolume(4, volume);
	SetVolume(5, volume);
	
	return true;
}

bool CSoundController::SetSFXVolume(const float volume)
{
	SetVolume(6, 0.5 * volume);
	//Player
	SetVolume(10, 0.1 * volume);
	SetVolume(12, volume);
	SetVolume(13, 0.25 * volume);
	//Enemy
	SetVolume(25, volume);
	SetVolume(26, volume);
	SetVolume(27, volume);

	return true;
}

float CSoundController::getCurrentVolume(const int ID)
{
	//Master
	if (ID == 0)
		return cSoundEngine->getSoundVolume();

	// Get the ISoundSource, if not exists, return 1.0 by default.
	CSoundInfo* SoundSource = GetSound(ID);
	if (SoundSource == nullptr)
	{
		return 1.0;
	}
	ISoundSource* pISoundSource = SoundSource->GetSound();
	if (pISoundSource == nullptr)
	{
		return 1.0;
	}

	// Get the current volume
	return pISoundSource->getDefaultVolume();
}


bool CSoundController::isPlaying(const int ID)
{
	CSoundInfo* pSoundInfo = GetSound(ID);
	if (!pSoundInfo)
	{
		return false;
	}
	
	return cSoundEngine->isCurrentlyPlaying(pSoundInfo->GetSound());
}

// For 3D sounds only
/**
 @brief Set Listener position
 @param x A const float variable containing the x-component of a position
 @param y A const float variable containing the y-component of a position
 @param z A const float variable containing the z-component of a position
 */
void CSoundController::SetListenerPosition(const float x, const float y, const float z)
{
	vec3dfListenerPos.set(x, y, z);
}

/**
 @brief Set Listener direction
 @param x A const float variable containing the x-component of a direction
 @param y A const float variable containing the y-component of a direction
 @param z A const float variable containing the z-component of a direction
 */
void CSoundController::SetListenerDirection(const float x, const float y, const float z)
{
	vec3dfListenerDir.set(x, y, z);
}

void CSoundController::SetSoundPosition(const float x, const float y, const float z, const int ID)
{
	CSoundInfo* pSoundInfo = GetSound(ID);
	pSoundInfo->SetPosition(x, y, z);
}

int CSoundController::GetSoundType(const int ID)
{
	CSoundInfo* pSoundInfo = GetSound(ID);
	if (pSoundInfo->GetSoundType() == CSoundInfo::SOUNDTYPE::_2D)
		return 2;
	else if (pSoundInfo->GetSoundType() == CSoundInfo::SOUNDTYPE::_3D)
		return 3;
	else
		return 0;
}

void CSoundController::SetSoundType(const int ID, int type)
{
	CSoundInfo* pSoundInfo = GetSound(ID);
	if (type == 2)
		pSoundInfo->SetSoundType(CSoundInfo::SOUNDTYPE::_2D);
	else if (type == 3)
		pSoundInfo->SetSoundType(CSoundInfo::SOUNDTYPE::_3D);
}

/**
 @brief Get an sound from this map
 @param ID A const int variable which will be the ID of the iSoundSource in the map
 @return A CSoundInfo* variable
 */
CSoundInfo* CSoundController::GetSound(const int ID)
{
	if (soundMap.count(ID) != 0)
		return soundMap[ID];

	return nullptr;
}

/**
 @brief Remove an sound from this map
 @param ID A const int variable which will be the ID of the iSoundSource in the map
 @return true if the sound was successfully removed, else false
 */
bool CSoundController::RemoveSound(const int ID)
{
	CSoundInfo* pSoundInfo = GetSound(ID);
	if (pSoundInfo != nullptr)
	{
		delete pSoundInfo;
		soundMap.erase(ID);
		return true;
	}
	return false;
}

/**
@brief Get the number of sounds in this map
@return The number of sounds currently stored in the Map
*/
int CSoundController::GetNumOfSounds(void) const
{
	return soundMap.size();
}