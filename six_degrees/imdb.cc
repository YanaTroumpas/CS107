using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

struct actorKey {
  const char* name;
  const void* data;
};

struct filmKey {
  const char* title;
  int year;
  const void* data;
};

static int actorComp(const void *actorData, const void *arrayOffset)
{
  const struct actorKey* actor = (const struct actorKey*)actorData;
  const int* offset = (const int*)arrayOffset;
  return strcmp(actor->name, (char*)actor->data + *offset);
}

static int filmComp(const void *filmData, const void *arrayOffset)
{
  const struct filmKey* film = (const struct filmKey*)filmData;
  const int* offset = (const int*)arrayOffset;

  char* title = (char*)film->data + *offset;
  int year = *((char*)film->data + *offset + strlen(title) + 1) + 1900;

  if (film->title == title) {
    if(film->year == year) return 0;
    else if(film->year < year) return -1;
    else return 1;
  }
  return strcmp(film->title, title);
}

int processPadding(const char* data) {
  int padding = 0;
  char c = *data;
  while(c == '\0') {
    data++;
    padding++;
    c = *data;
  }
  return padding;
}


imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}


bool imdb::getCredits(const string& player, vector<film>& films) const { 

  // actorFile begins with an integer count followed by an array of offsets
  int* total_num = (int*)actorFile;
  int* offsets = (int*)actorFile + 1;

  struct actorKey key;
  key.name = player.c_str();
  key.data = actorFile;

  // Look up the actor record using binary search over the offset table.
  void* found = bsearch(&key, offsets, *total_num, sizeof(int), actorComp);
  if (found == NULL){
    return false;
  } 

  int* found_offset = (int*) found;  
  char* name = (char*)actorFile + *found_offset;
  int name_len = player.size();
  
  name_len += processPadding(name + name_len);

  // Find the number of movies for each actor
  void* actorInfo = (char*) actorFile + *found_offset + name_len*sizeof(char);
  short* num_movies = (short*) actorInfo;
  cout << "Num movies: " << *num_movies << endl;
  actorInfo = (short*) actorInfo + 1;

  // skip padding after num of movies
  int padding = processPadding((char*) actorInfo);
  actorInfo = (char*) actorInfo + padding;
  
  for(int i = 0; i < *num_movies; i++) {
    int* movie_offset = (int*)actorInfo + i;
    char* movie = (char*) movieFile + *movie_offset;
    string movie_title(movie);
    int title_len = movie_title.size();
    
    title_len += processPadding(movie + title_len);
  
    int year = *((char*) movieFile + *movie_offset + title_len + 1) + 1900;
    films.push_back({movie_title, year});
  }

  return true;
}

bool imdb::getCast(const film& movie, vector<string>& players) const { 

  // movieFile begins with an integer count followed by an array of offsets
  int* total_num = (int*)movieFile;
  int* offsets = (int*)movieFile + 1;

  struct filmKey key;
  key.title = movie.title.c_str();
  key.year = movie.year;
  key.data = movieFile;

  // Look up the movie record using binary search over the offset table.
  void* found = bsearch(&key, offsets, *total_num, sizeof(int), filmComp);
  if (found == NULL){
    return false;
  }

  int* found_offset = (int*) found;  
  // skip over title and year to get to the number of actors
  char* filmInfo = (char*)movieFile + *found_offset + movie.title.size() + 2;
  int padding = processPadding(filmInfo);

  filmInfo = (char*) filmInfo + padding;
  short* num_actors = (short*) filmInfo;
  cout << "Num actors: " << *num_actors << endl;

  void* castInfo = (short*) filmInfo + 1;

  // skip padding after num of movies
  char* actorList = (char*) castInfo;
  padding = processPadding(actorList);

  castInfo = (char*) castInfo + padding;

  for(int i = 0; i < *num_actors; i++) {
    int* actor_offset = (int*) castInfo + i;
    players.push_back(string((char*) actorFile + *actor_offset));
  }

  return true; }

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
