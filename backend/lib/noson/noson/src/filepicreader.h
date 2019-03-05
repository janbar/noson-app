#ifndef FILEPICREADER_H
#define FILEPICREADER_H

#include "local_config.h"

#include "streamreader.h"

#include <string>
#include <vector>

#define FILEPICREADER_PARAM_PATH  "path"
#define FILEPICREADER_PARAM_TYPE  "type"

namespace NSROOT
{

class FilePicReader : public StreamReader
{
public:
  struct Picture
  {
    void * payload;
    void (*free)(void * payload);
    const char * mime;
    const char * data;
    unsigned size;
    Picture();
    ~Picture();
  };
  static FilePicReader _instance;
  FilePicReader();

public:
  virtual ~FilePicReader() override { }
  static FilePicReader * Instance();

  STREAM * OpenStream(const std::string& streamUrl) override;
  int ReadStream(STREAM * stream) override;
  void CloseStream(STREAM * stream) override;

  enum PictureType // according to the ID3v2 APIC frame
  {
    Any           =-1,
    Other         = 0,
    fileIcon      = 1,
    OtherFileIcon = 2,
    CoverFront    = 3,
    CoverBack     = 4,
    LeafletPage   = 5,
    Media         = 6,
    LeadArtist    = 7,
    Artist        = 8,
    Conductor     = 9,
    Orchestra     = 10,
    Composer      = 11,
    Lyricist      = 12,
  };

private:
  static void readParameters(const std::string& streamUrl, std::vector<std::string>& params);
  static std::string getParamValue(const std::vector<std::string>& params, const std::string& name);

  static Picture * ExtractFLACPicture(const std::string& filePath, PictureType pictureType, bool& error);
  static void FreeFLACPicture(void * payload);

  static Picture * ExtractID3Picture(const std::string& filePath, PictureType pictureType, bool& error);
  static void FreeID3Picture(void * payload);
  static int parse_id3v2(FILE * file, long id3v2_offset, Picture ** pic, off_t * ptag_size, PictureType pictureType);
  static long find_id3v2(FILE * file, off_t * sync_offset);
  static int parse_id3v2_pic_v2(FILE * file, unsigned frame_size, Picture ** pic, PictureType pictureType);
  static int parse_id3v2_pic_v3(FILE * file, unsigned frame_size, Picture ** pic, PictureType pictureType);
};

}

#endif /* FILEPICREADER_H */

