
class PlatformCategory
{

public:
	enum PlatformType
	{
		UNLISTED_VALUE,
		AIRCRAFT,
		TORPEDO
	};
	/*
	enum Destination
	{
	UNLISTED_VALUE,
	NO_VALUE,
	ALDERAAN
	};

	enum Affinity
	{	
	UNLISTED_VALUE,
	NO_VALUE,
	FRIENDLY
	};*/

	//constructor
	PlatformCategory();

	~PlatformCategory();

	//Get and Set functions
	char *getPlatformTypeStr(PlatformType type);
	char *getAffinityStr();
	char *getDestinationStr();

private:
	PlatformType platformType_;
	//Destination destination_;
	//Affinity affinity_;
};