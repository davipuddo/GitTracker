#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>

class ByteArray
{
	public:
	uint8_t* data;
	int size;

	ByteArray(int n)
	{
		data = NULL;
		size = -1;

		if (n > 0)
		{
			data = new uint8_t[n];
			size = n;
		}
		else
		{
			std::cout << "ERROR: Invalid size!\n";
		}
	}

	ByteArray(uint8_t* other, int n)
	{
		data = NULL;
		size = -1;
		if (other && n > 0)
		{
			data = other;
			size = n;
		}
		else
		{
			std::cout << "ERROR: Invalid data!\n";
		}
	}

	void print (void)
	{
		if (data && size > 0)
		{
			for (int i = 0; i < size; i++)
			{
				printf("[%x] ", data[i]);
			}
			std::cout << "\n";
		}
	}
};

class DataTypes
{
	private:

	ByteArray* BA;
	int index;

	void initST (uint8_t* data, int s, int i)
	{
		BA = NULL;
		index = -1;

		if (data && i > -1 && s > 0)
		{
			BA = new ByteArray(data, s);
			index = i;
		}
		else
		{
			std::cerr << "ERROR: Invalid data!\n";
		}
	}

	void initOB (ByteArray* BA, int i)
	{
		this->BA = NULL;
		index = -1;

		if (BA && i > -1 && i < BA->size)
		{
			this->BA = BA;
			index = i;
		}
		else
		{
			std::cerr << "ERROR: Invalid data!\n";
		}
	}

	public:

	DataTypes (uint8_t* BA, int s, int i)
	{
		initST(BA, s, i);
	}

	DataTypes (uint8_t* BA, int s)
	{
		initST(BA, s, 0);	
	}

	DataTypes (int s)
	{
		if (s > 0)
		{
			uint8_t* aux = new uint8_t[s];
			initST(aux, s, 0);
		}
		else
		{
			std::cerr << "ERROR: Invalid size!\n";
		}
	}

	DataTypes (ByteArray* BA, int i)
	{
		initOB(BA, i);
	}

	DataTypes (ByteArray* BA)
	{
		initOB(BA, 0);
	}

	~DataTypes ()
	{
		delete (BA);
		index = -1;
	}

	void seek (int i)
	{
		if (i < BA->size)
		{
			index = i;
		}
	}

	int getIndex (void)
	{
		return (index);
	}

	ByteArray* getByteArray (void)
	{
		ByteArray* res = NULL;
		if (BA)
		{
			res = new ByteArray(BA->size);

			for (int i = 0; i < BA->size; i++)
			{
				res->data[i] = BA->data[i];
			}
		}
		return (res);
	}

	void writeInt (int x)
	{
		if (BA)
		{
			BA->data[index++] = (0xFF000000 & x) >> 24;
			BA->data[index++] = (0x00FF0000 & x) >> 16;
			BA->data[index++] = (0x0000FF00 & x) >> 8;
			BA->data[index++] = (0x000000FF & x);
		}
	}

	void writeUTF (const char* str)
	{
		if (BA && str)
		{
			int n = (int)strlen(str);

			writeInt (n);

			for (int i = 0; i < n; i++)
			{
				BA->data[index+i] = (uint8_t)str[i];
			}
			index += n;
		}
	}

	int readInt (void)
	{
		int res = -1;
		if (BA)
		{
			res = ( (BA->data[index] << 24)| (BA->data[index+1] << 16) | (BA->data[index+2] << 8) | BA->data[index+3]);	
			index += 4;
		}
		else
		{
			std::cerr << "ERROR: Invalid data!\n";
		}
		return (res);
	}

	char* readUTF (void)
	{
		char* res = NULL;
		if (BA)
		{
			int n = readInt();

			if (n > 0)
			{
				res = new char[n];
				
				for (int i = 0; i < n; i++)
				{
					res[i] = BA->data[index++];
				}
			}
		}
		else
		{
			std::cerr << "ERROR: Invalid data!\n";
		}
		return (res);
	}

	void print ()
	{
		if (BA)
		{
			BA->print();
		}
	}
};

class Cell
{
	public:
	char* name;
	char* path;
	Cell* link;

	Cell (const char* const name, const char* const path)
	{
		this->name = NULL;
		this->path = NULL;
		this->link = NULL;

		if (!name || !path)
		{
			std::cerr << "ERROR: Invalid parameter!\n";
		}
		else
		{
			this->name = (char*)calloc(strlen(name)+1, sizeof(char));
			this->path = (char*)calloc(strlen(path)+1, sizeof(char));

			strcpy(this->name, name);
			strcpy(this->path, path);
		}
	}

	~Cell ()
	{
		link = NULL;

		delete(name);
		delete(path);
	}

	void print (void)
	{
		if (name && path)
		{
			std::cout << "Name: " << name << "\n";
			std::cout << "Path: " << path << "\n";
		}
		else
		{
			std::cerr << "ERROR: Invalid data!\n";
		}
	}
};

class Queue
{
	private:
	Cell* head;
	Cell* tail;
	int n;

	public:
	Queue ()
	{
		head = NULL;
		tail = NULL;
		n = 0;
	}

	~Queue ()
	{
		Cell* ptr = head;
		while (ptr)
		{
			Cell* buf = ptr;
			ptr = ptr->link;
			delete (buf);
		}

		if (tail)
		{
			delete (tail);
		}

		n = 0;
	}

	void insert (const char* const name, const char* const path)
	{
		if (!name || !path)
		{
			std::cerr << "ERROR: Invalid parameters!\n";
		}
		else
		{
			if (!head)
			{
				head = new Cell(name, path);
				tail = head;
			}
			else
			{
				tail->link = new Cell (name, path);
				tail = tail->link;
			}
			n++;
		}
	}

	Cell* remove (void)
	{
		Cell* res = NULL;
		if (!head)
		{
			std::cerr << "ERROR: The queue is empty!\n";
		}	
		else
		{
			res = head;
			head = head->link;
			n--;
		}
		return (res);
	}

	void print (void)
	{
		for (Cell* ptr = head; ptr; ptr = ptr->link)
		{
			std::cout << " -----\n";
			ptr->print();
			std::cout << " -----\n";
		}
		std::cout << "\n===========\n\n";
	}

	ByteArray* toByteArray (void)
	{
		ByteArray* res = NULL;
		if (head)
		{
			int totalSize = 0;
			for (Cell* ptr = head; ptr; ptr = ptr->link)
			{
				if (ptr->name && ptr->path)
				{
					totalSize += strlen(ptr->name) + strlen(ptr->path) + 8;
				}
			}

			totalSize += 4;

			DataTypes* DT = new DataTypes(totalSize);

			DT->writeInt(n);

			for (Cell* ptr = head; ptr; ptr = ptr->link)
			{
				DT->writeUTF(ptr->name);
				DT->writeUTF(ptr->path);
			}

			res = DT->getByteArray();
		}
		return (res);
	}

	void fromByteArray (ByteArray* BA)
	{
		if (BA)
		{
			DataTypes* DT = new DataTypes(BA);
			
			int n = DT->readInt();

			for (int i = 0; i < n; i++)
			{
				char* NameBuf = DT->readUTF();
				char* PathBuf = DT->readUTF();

				insert(NameBuf, PathBuf);
			}
		}
		else
		{
			std::cerr << "ERROR: Invalid object!\n";
		}
	}

};

class Log
{
	private:
	const char* filename;
	const short HEADER_SIZE = 1;

	public:
	Log (const char* filename)
	{
		this->filename = NULL;

		if (filename)
		{
			this->filename = filename;
		}
	}

	void writeFile (ByteArray* BA)
	{
		if (!BA)
		{
			std::cerr << "ERROR: Invalid data!";
		}
		else
		{
			std::ofstream fs (filename, std::ios::binary);

			uint8_t b = (BA->size & 0x7F000000);
			fs << b;
			b = (BA->size & 0x00FF0000);
			fs << b;
			b = (BA->size & 0x0000FF00);
			fs << b;
			b = (BA->size & 0x000000FF);
			fs << b;

			for (int i = 0; i < BA->size; i++)
			{
				fs << BA->data[i];
			}

			fs.close();
		}
	}

	ByteArray* readFile (void)
	{
		ByteArray* res = NULL;

		std::ifstream fs (filename, std::ios::binary);

		uint8_t buf [4];

		for (int i = 0; i < 4; i++)
		{
			fs >> buf[i];
		}

		int size = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]));

		res = new ByteArray(size);

		for (int i = 0; i < size; i++)
		{
			fs >> res->data[i];
		}

		fs.close();

		return (res);	
	}
};

class Tracker
{
	private:
	Queue* repos;

	public:

	Tracker ()
	{
		repos = NULL;
	}

	Tracker (const char* const filename)
	{
		//repos = fromFile (filename);
	}

	char* commandOutput (char* command)
	{
		// Define variables
		char* res = NULL;
		FILE* fp = NULL;
		int LCount = 0;
		int n = 100;

		// Alloc array
		res = (char*)calloc(n+1, sizeof(char));

		// Open stream
		fp = popen (command, "r");

		if (fp)
		{
			// Alloc auxiliar array
			char* aux = (char*)calloc(81,sizeof(char));

			while (fgets(aux, 80, fp)) // Read command output
			{
				LCount += strlen(aux); // Count output characters

				// Increase result array size
				if (LCount > n)
				{
					n = (n*2);
					res = (char*)realloc(res, (n+1)*sizeof(char));
				}

				// Error check
				if (!res)
				{
					std::cerr << "ERROR: Not enough memory!\n";
				}
				else
				{
					strcat(res, aux);
				}
			}

			// Free array
			free(aux);

			//std::cout << "LCount: " << LCount << "\n";
			//std::cout << "n: " << n << "\n";

			// Close stream
			pclose(fp);
		}
		return (res);
	}

	bool match (const char* const line, const char* const target)
	{
		bool res = false;
		int lineN = strlen(line);
		int targN = strlen(target);

		if (lineN < targN)
		{
			std::cerr << "ERROR: Invalid string comparison!\n";
		}
		else if (lineN == targN)
		{
			res = (strcmp(line, target) == 0);	
		}
		else
		{
			int n = lineN;
			int i = 0;
			int y = 0;

			while (i < lineN && y < targN)
			{
				if (line[i] == target[y])
				{
					y++;
				}
				else
				{
					if (i >= (lineN - targN))
					{
						i = lineN;
					}
					y = 0;
				}

				i++;
			}

			if (y == targN)
			{
				res = true;
			}
		}
		return (res);
	}
};

int main (void)
{

	Queue* q = new Queue();
	q->insert("obs", "~/repos/obs");
	q->insert("8085asm", "~/repos/8085asm");
	q->print();
	ByteArray* BA = q->toByteArray();

	Queue* q2 = new Queue();

	q2->fromByteArray(BA);
	q2->print();

	Log* l = new Log("log.bin");

	l->writeFile(BA);
	ByteArray* f = l->readFile();

	BA->print();
	printf("\n-----------\n\n");
	f->print();

	return (0);
}
