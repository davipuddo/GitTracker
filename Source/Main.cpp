#include <iostream>
#include <cstring>
#include <fstream>
#include <string>

class List
{
	public:

	// Atributes
	char** data;
	int length;

	// Constructor
	List (int n)
	{
		data = NULL;
		length = 0;

		if (n > 0)
		{
			data = new char* [n+1];
			for (int i = 0; i < n; i++)
			{
				data[i] = NULL;
			}
			length = n;
		}
		else
		{
			std::cerr << "ERROR: Invalid list size!\n";
		}
	}

	// Distructor
	~List ()
	{
		if (data)
		{
			for (int i = 0; i < length; i++)
			{
				if (data[i])
				{
					delete (data[i]);
				}
			}
			delete(data);
		}
	}

	// Other methods

	void insert (char* x, int pos)
	{
		if (x && pos < length)
		{
			if (data[pos])
			{
				delete(data[pos]);
			}
			data[pos] = new char[strlen(x)+1];
			strcpy(data[pos], x);
		}
	}

	void print (void)
	{
		for (int i = 0; i < length; i++)
		{
			std::cout << data[i] << "\n";
		}
	}
};

class Repo
{
	public:

	// Atributes
	char* name;
	char* path;

	// Constructors // 

	Repo (char* path)
	{
		if (path)
		{
			this->path = new char[strlen(path)+1];
			strcpy(this->path, path);
		}
		this->name = NULL;
	}

	Repo ()
	{
		this->path = NULL;
		this->name = NULL;
	}

	// Distructor
	~Repo ()
	{
		if (this->path)
		{
			delete(path);
		}
		if (this->name)
		{
			delete(name);
		}
	}

	// Other methods //

	// Recover repo's name from the path
	void getName (void)
	{
		int n = strlen(path);
		int i = n;
		int nameS = 0;

		// Find repo's name size
		while (i > -1 && path[i] != '/' || path[i] != '\\') 
		{
			nameS++;
			i--;
		}
		nameS--;

		name = new char[nameS+1];

		i = n-nameS;
		int j = 0;
		
		// Copy name
		while (j < nameS && i < n)
		{
			name[j++] = path[i++];
		}
		name[nameS] = '\0'; // Add '\0' at the end
	}

	void print (void)
	{
		std::cout << "Name: " << name << "\nPath: " << path << "\n";
	}
};

// Read a command's output
std::string commandOutput (char* command)
{
	// Define variables
	std::string res = "";
	FILE* fp = popen(command, "r");

	if (fp)
	{
		char* buf = new char[81];
		int lineC = 0;
		int n = 100;

		// Read output
		while (fgets(buf, 80, fp))
		{
			res += buf; // Concat string
		}

		// Free buffer
		if (buf)
		{
			delete(buf);
		}
	}
	return (res);
}

// Get file's line count
int fileSize (std::fstream* fs)
{
	int res = 0;
	if (fs && *fs) // Check stream
	{
		// Go to the end
		fs->seekg(0, std::ios::end);

		// Get pointer position
		int end = (int)fs->tellg();

		// Go to the start
		fs->seekg(0, std::ios::beg);

		// Iterate
		while (fs->tellg() < end)
		{
			if (fs->get() == '\n') // Count '\n'
			{
				res++;
			}
		}
		fs->seekg(0); // Go back to the start
	}
	return (res);
}

List* readFile (void)
{
	// Define variables
	List* list = NULL;
	std::string line = "";
	int n = 0;
	int i = 0;

	// Open file stream
	std::fstream fs ("repos.db", std::ios::in);

	if (fs) // Check file status
	{
		// Read repos number
		n = fileSize(&fs);
			
		// Create new list
		list = new List(n);

		// Read file
		while (i < n && std::getline(fs, line))
		{
			if (line.data())
			{
				list->insert(line.data(), i++);
			}
		}

		// Close stream
		fs.close();
	}
	else
	{
		std::cerr << "ERROR: The file could not be opened!\n";
	}

	return (list);
}

Repo** getRepos (List* list)
{
	Repo** repos = NULL;

	if (list)
	{
		// Get repos number
		int n = list->length;

		repos = new Repo*[n];

		if (repos)
		{
			// Read data
			for (int i = 0; i < n; i++)
			{
				repos[i] = new Repo(list->data[i]);
				repos[i]->getName();
			}
		}
	}
	return (repos);
}

int main (void)
{
	// Read file
	List* file = readFile();

	// Define repos
	Repo** repos = getRepos(file);

	// Get repos size
	int n = file->length;

	// Get status
	for (int i = 0; i < n; i++)
	{
		// Get string
		std::string name(repos[i]->name);
		std::string path(repos[i]->path);
		
		std::cout << "======= " << name << " =======\n"; 

		// Fetch repo
		std::cout << "fetching...\n";
		std::string line = "git -C " + path + " fetch";
		std::string fetch = commandOutput(line.data());

		// Get status
		std::cout << "Status: ";
		line = "git -C " + path + " status -sb";
		std::string status = commandOutput(line.data());

		// Define target
		std::string target = "[behind";

		// Define variables
		int ln = status.length();
		int tgn = target.length();
		int y = 0;
		int c = 0;

		// Compare status with the target
		while (y < ln && c < tgn)
		{
			// Read only the first line
			if (status[y] == '\n')
			{
				y = ln; // Exit loop
			}
			else if (status[y] == target[c]) // Compare chars
			{
				c++;
			}
			else // Not a match
			{
				c = 0; // Restart position

				if (y == (ln-tgn)) // If there is no space for the target
				{
					y = ln; // Exit loop
				}
			}
			y++;
		}
	
		// Print status
		if (c == tgn)
		{
			std::cout << "Behind!\n";
		}
		else
		{
			std::cout << "OK!\n";
		}
	}

	// Free data
		// List
	if (file)
	{
		delete(file);
	}

		// Repos
	for (int i = 0; i < n; i++)
	{
		if (repos[i])
		{
			delete(repos[i]);
		}
	}
	if (repos)
	{
		delete(repos);
	}

	return (0);
}
