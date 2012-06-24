#ifndef _SH_GGPSOL_H_
#define _SH_GGPSOL_H_

#include <vector>
#include <string>

class argAnalyzer {
private :
   static const std::string flags;
   static const int defaultFlag;
   static const std::string helpMessage;

   const std::string executableName;
   const std::string executablePrompt;
   std::vector<std::string> vecInputFileNames;
   std::vector<bool> isOn;

   bool isThereError;
   std::string errorMessage;

   inline bool isFlag( const std::string & s ) const {
      return ( s.length() != 0 && s[0]== '-' );
   }

   void analyzeFlag( const std::string & );

public :
   argAnalyzer( int argc, char * argv [] );

   bool isHelpOn() const             {
      return isOn[0];
   }
   bool isGGPToStandardOutOn() const {
      return isOn[1];
   }
   bool isGPToStandardOutOn() const  {
      return isOn[2];
   }
   bool isGGPToFileOn() const        {
      return isOn[3];
   }
   bool isGPToFileOn() const         {
      return isOn[4];
   }
   bool isSolveDualGPOn() const      {
      return isOn[5];
   }
   //bool isMatlabOn() const           { return isOn[5]; }
   //bool isMakeMatlabFileOn() const   { return isOn[7]; }

   bool hasAnyErrorOccurred() const {
      return isThereError;
   }

   const std::string getHelpMessage() const {
      return
         ( "usage: " + executableName + " [-" + flags + "] [file ...]\n"
           + helpMessage );
   }

   const std::string & getErrorMessage() const {
      return errorMessage;
   }

   const std::vector<std::string> & getInputFileNames() const {
      return vecInputFileNames;
   }

   int getNumberOfInputFiles() const {
      return vecInputFileNames.size();
   }
};

#endif
