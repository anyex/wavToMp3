#include <cstdio>
#include <cstdio>
#include <sstream>
#include <memory>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <cstring>
#include <algorithm>
#include <thread>
#include <set>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/sem.h>
#include <dirent.h>
#include <string.h>


int processNum = 0;
char swav[128] = { 0 };
/*�Զ���string��*/
class MyString {
public:
	MyString(char* s) :str(s) {  };
	MyString() :str() {};
	MyString(std::string&A) :str(A) {};

	void  DeleteMark(const char *mark) {

		str.erase(std::remove(str.begin(), str.end(), *mark), str.end());

	}

	void  Split(std::string &s1, std::string &s2, const std::string &flag)
	{
		int pos = str.find(flag);
		if (pos != -1)
		{
			s2 = str.substr(pos + 1, str.size() - 1);
			s1 = str.substr(0, pos);
		}
	}

	void Split(std::set<std::string>& s, const std::string&c) {
		std::string::size_type pos1, pos2;
		pos2 = str.find(c);
		pos1 = 0;
		while (std::string::npos != pos2)
		{
			std::string t = str.substr(pos1, pos2 - pos1);
			if (t.find("-"))
			{
				break;
			}
			s.insert(t);
			pos1 = pos2 + c.size();
			pos2 = str.find(c, pos1);

		}
		if (pos1 != str.length())
			s.insert(str.substr(pos1));

	}

	friend std::ostream& operator<<(std::ostream&, MyString&);
	int find(const std::string &s)
	{
		return str.find(s);
	}

	std::string str;

};
/*�ļ�ϵ�д���*/
class File {
public:
	/*��������·�������·��*/
	File(std::string &Path,std::string &output) :path(Path),out(output){
		readFile();
	};


	/*����������*/
	static void Rename(char *oldname, char *flag)
	{
		char newName[128], name[128];
		sscanf(oldname, "%[^.]", name);
		sprintf(newName, "%s%s%s", name, ".wav", flag);
		printf("%s", newName);
		rename(oldname, newName);
	}

	//�ָ��ļ�·�����ļ���(���ļ�·�����ļ�������չ����)
	void splitpath(const char *path, char*dir, char* fname,char*ext)
	{
		if (path == NULL)
		{
			dir[0] = '\0';
			fname[0] = '\0';
			return;
		}

		if (path[strlen(path)] == '/')
		{
			strcpy(dir, path);
			fname[0] = '\0';
			return;
		}
		char *whole_name = (char *)rindex(path, '/');

		printf("whole_name:  %s\n", whole_name + 1);

		snprintf(dir, path - whole_name, "%s", path);



		if (whole_name != NULL)
		{
			char *pext = rindex(whole_name, '.');

			if (pext != NULL)
			{
				strcpy(ext, pext + 1);
				int ret = whole_name - pext;
				snprintf(fname, pext - whole_name, "%s", whole_name + 1);
			}
			else
			{
				ext[0] = '\0';
				strcpy(fname, whole_name);
			}
		}

	}
	
	/*�����ļ�*/
	void readFile()
	{
		
		DIR *input = opendir(path.c_str());
		std::string file;
		struct dirent *direntp;
		if (input)
		{
			while ((direntp = readdir(input)) != NULL)
			{
				if (direntp->d_type == 8)//�ļ�
				{
					std::string filename(direntp->d_name);
					if (filename.find(".wav-") == -1)
					{
						file.append(path);
						file.append("/");
						file.append(direntp->d_name);

						if (access(file.c_str(), R_OK))//�ж��ļ��Ƿ�ɶ�
						{
							set.insert(file);
							
						}
							file.clear();
							continue;
						
						//if (set.size() >= 20)
							//sleep(5);
					}
					else
						continue;

				}
				if (direntp->d_type == 4)//Ŀ¼
				{
					std::string s = direntp->d_name;

					if (s.find(".") == -1)
					{
						std::string Path = path;
						path.append("/");
						path.append(direntp->d_name);
						readFile();
						path = Path;
					}else
						continue;
				}
			}
		}
	}

	/*����һ����ת����wav�ļ�·����ת���������ļ�·��*/
	bool get()
	{
		if (!set.empty())
		{
			memset(wav,0, sizeof(wav));
			memset(mp3, 0, sizeof(mp3));

			std::string file = *set.begin();
			strcpy(wav, file.c_str());

			int a = file.find_last_of("/");
			int b = file.find_last_of(".");

			file = file.substr(a + 1, b - a - 1);
			
			//strcpy(name, file.c_str());

			sprintf(mp3,"%s%s%s",out.c_str(), file.c_str(),".mp3");

			set.erase(set.begin());

			return true;
		}
	
			return false;
	}


	/*ִ��ת��*/
	void DoTrans() {
		processNum++;
		char *cmd[] = { "--preset fast standard",wav,mp3,NULL };
		if (execv("/usr/bin/lame", cmd) <0)
		{
			perror("error on exec");
			exit(0);
		}
	}

	/*�жϼ����б��Ƿ�Ϊ��*/
	bool IsEmptySet()
	{
		return set.empty();
	}
	char wav[128];//��ת����wav�ļ�(·��+�ļ���)
private:
	char mp3[128];//ת�����mp3�ļ�(·��+�ļ���)
	
	char name[128];//�ļ�����
	std::set<std::string> set;//��ȡ���ļ��б�
	std::string & path;//�����ļ�·��
	std::string & out; //����ļ�·��
};

/*�����ӽ���*/
void sig_handle(int num)
{
	int status;
	pid_t pid;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		if (WIFEXITED(status))
		{
			printf("child process exit pid[%6d],exit code[%d]\n", pid, WEXITSTATUS(status));
			processNum--;
			File::Rename(swav, "-");
		}
		else {
			printf("child process exit but...\n");

		}
	}
}

/*��ȡ�����ļ�*/
class Ini {

public:
	Ini(const std::string& file) :filename(file) {
		getIni();
	};
	void getIni() {
		std::ifstream inflie(filename.data());
		if (inflie)
		{
			MyString s;
			std::string s1, s2, root;
			while (getline(inflie, s.str))
			{
				int pos = s.find("//");
				if (pos == 0)
				{
					continue;
				}
				s.DeleteMark(" ");
				s.DeleteMark("\r");
				s.DeleteMark("\t");

				if (s.find("[") != -1)
				{
					s.DeleteMark("[");
					s.DeleteMark("]");
					root = s.str;
					continue;
				}
				s.Split(s1, s2, "=");
				Inid[root][s1] = s2;
			}
		}
		else
		{
			std::cout << "can't open file!" << std::endl;
		}
		inflie.close();
	};
public:

	std::map <std::string, std::map<std::string, std::string> > Inid;
	const std::string& filename;
};

int main()
{
	

	Ini ini("/root/task.ini");
	File file(ini.Inid["set"]["input"], ini.Inid["set"]["output"]);
	
	
	signal(SIGCHLD, sig_handle);//Ϊ�ӽ����˳�ע���¼�

	while (true)
	{
		if (processNum < 4&&(!file.IsEmptySet()))
		{
			
			if (vfork() == 0)
			{
				//processNum++;
				if (!file.get())
					exit(0);
				strcpy(swav, file.wav);
				file.DoTrans();
			}
			else
			{
				continue;
			}
		}
		else if(processNum<4)
		{
			sleep(1);
		}
		else {
			file.readFile();
		}
	}
    return 0;
}

