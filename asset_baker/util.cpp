bool readFile(const char* filePath, std::vector<char>& fileBytes) {
  //open the file. With cursor at the end
  std::ifstream file(filePath, std::ios::ate | std::ios::binary);

  if(!file.is_open()) {
    std::cout << "Could not open file: " << filePath << std::endl;
    return false;
  }

  //find what the size of the file is by looking up the location of the cursor
  u64 fileSize = (u64)file.tellg();

  fileBytes.clear();
  fileBytes.resize(fileSize);

  //put file cursor at beginning
  file.seekg(0);
  file.read((char*)fileBytes.data(), fileSize);
  file.close();

  return true;
}

void writeFile(const char* filePath, const std::string& fileBytes) {
  std::ofstream outfile;
  outfile.open(filePath, std::ios::binary | std::ios::out);

  u32 fileLength = static_cast<u32>(fileBytes.size());
  outfile.write(fileBytes.data(), fileLength);

  outfile.close();
}

char* intToStr(int value, char* buffer) { // base 10
  char* charPtr = buffer;

  // record digits least to most significant
  while(char digit = (value % 10)) {
    char c = digit + '0';
    *charPtr = c;
    charPtr++;
    value = value / 10;
  }

  // null terminate str
  *charPtr = '\0';

  // reverse digit order
  char* end = charPtr - 1;
  char* start = buffer;
  while(start < end) {
    char tmp = *start;
    *start = *end;
    *end = tmp;
    start++;
    end--;
  }

  return buffer;
}