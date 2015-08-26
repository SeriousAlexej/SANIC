flexc++ -l objScanner.cpp -i objScanner.impl.h -c objScanner.h -b objScanner.base.h objScanner.l
bisonc++ -p objParser.cpp -i objParser.impl.h -c objParser.h -b objParser.base.h objParser.y

