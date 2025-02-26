#include <iostream>
#include <cstring>
#include <fstream>
#include <string>

class List
{
	public:
	std::string* data;
	int length;

	List (int n)
	{
		data = NULL;
		length = 0;

		if (n > 0)
		{
			data = new std::string [n];
			length = n;
		}
		else
		{
			std::cerr << "ERROR: Invalid list size!\n";
		}
	}

	void print ()
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
	std::string name;
	std::string path;

	Repo (std::string name, std::string path)
	{
		this->name = name;
		this->path = path;
	}

	Repo ()
	{
		name = "";
		path = "";
	}

	void print (void)
	{
		std::cout << "Name: " << name << "\nPath: " << path << "\n";
	}
};

std::string commandOutput (char* command)
{
	// Define variables
	std::string res = "";
	FILE* fp = popen(command, "r");

	if (fp)
	{
		char* buf = new char[80];
		int lineC = 0;
		int n = 100;

		// Read output
		while (fgets(buf, 80, fp))
		{
			res += buf; // Concat string
		}

		// Free buffer
		delete(buf);
	}
	return (res);
}

List* readFile (void)
{
	// Define variables
	List* res = NULL;
	std::string line = "";
	int n = 0;
	int i = 0;

	// Open file stream
	std::fstream fs ("repos.db", std::ios::in);

	if (fs) // Check file status
	{
		// Read repos number
		fs >> n;
			
		// Convert to line number
		n *= 2;

		// Create new list
		res = new List(n);

		// Skip first line
		std::getline(fs, line);

		// Read file
		while (std::getline(fs, line))
		{
			res->data[i] = line;
			i++;
		}

		// Close stream
		fs.close();
	}
	else
	{
		std::cerr << "ERROR: The file could not be opened!\n";
	}

	return (res);
}

Repo* getRepos (List* list)
{
	Repo* res = NULL;

	if (list)
	{
		// Get repos number
		int n = (int)(list->length/2.0);

		res = new Repo[n];

		if (res)
		{
			// Read data
			for (int i = 0; i < n; i++)
			{
				int x = (2*i);
				res[i] = Repo(list->data[x], list->data[x+1]);
			}
		}
	}
	return (res);
}

int main (void)
{
	// Read file
	List* file = readFile();

	// Define repos
	Repo* repos = getRepos(file);

	// Get repos size
	int n = (file->length/2.0);

	// Get status
	for (int i = 0; i < n; i++)
	{
		std::cout << "======= " << repos[i].name << " =======\n"; 

		// Fetch repo
		std::cout << "fetching...\n";
		std::string line = "git -C " + repos[i].path + " fetch";
		std::string fetch = commandOutput(line.data());

		// Get status
		std::cout << "Status: ";
		line = "git -C " + repos[i].path + " status -sb";
		std::string status = commandOutput(line.data());

		// Read status
		std::string target = "[behind";

		// Define variables
		int ln = status.length();
		int tgn = target.length();
		int y = 0;
		int c = 0;

		while (y < ln && c < tgn)
		{
			// Read only the first line
			if (status[y] == '\n')
			{
				y = ln;
			}
			else if (status[y] == target[c])
			{
				c++;
			}
			else
			{
				c = 0;
				if (y == (ln-tgn))
				{
					y = ln;
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

	return (0);
}
