
/** mapInfo - A struct containting mapping information for the createfiles and other functions. 
*	This should be populated with the running simulation's mapping info.
*	*/

typedef struct MapInfo
{
	uint32_t numCores;
	uint32_t coresPerPE;
	uint32_t coreSize;
}mapInfo;

/** createFiles - creates a series of files based on the mapping of the simulation. Expects
*	to find a spikes.csv file in the running directory. Given the number of cores per PE, 
*	this function splits the file into seperate spike files so that each PE can access it's own
*	CSV file independantly. Saves the files as spikes_nnnn.csv, where nnnn is the PE ID where the 
*	spikes are located.  */
void createFiles(mapInfo map);

void 