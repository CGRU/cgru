/**************************************************************************************

 Copyright (C) 2001 - 2008 Autodesk, Inc. and/or its licensors.
 All Rights Reserved.

 The coded instructions, statements, computer programs, and/or related material 
 (collectively the "Data") in these files contain unpublished information 
 proprietary to Autodesk, Inc. and/or its licensors, which is protected by 
 Canada and United States of America federal copyright law and by international 
 treaties. 
 
 The Data may not be disclosed or distributed to third parties, in whole or in
 part, without the prior written consent of Autodesk, Inc. ("Autodesk").

 THE DATA IS PROVIDED "AS IS" AND WITHOUT WARRANTY.
 ALL WARRANTIES ARE EXPRESSLY EXCLUDED AND DISCLAIMED. AUTODESK MAKES NO
 WARRANTY OF ANY KIND WITH RESPECT TO THE DATA, EXPRESS, IMPLIED OR ARISING
 BY CUSTOM OR TRADE USAGE, AND DISCLAIMS ANY IMPLIED WARRANTIES OF TITLE, 
 NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE OR USE. 
 WITHOUT LIMITING THE FOREGOING, AUTODESK DOES NOT WARRANT THAT THE OPERATION
 OF THE DATA WILL BE UNINTERRUPTED OR ERROR FREE. 
 
 IN NO EVENT SHALL AUTODESK, ITS AFFILIATES, PARENT COMPANIES, LICENSORS
 OR SUPPLIERS ("AUTODESK GROUP") BE LIABLE FOR ANY LOSSES, DAMAGES OR EXPENSES
 OF ANY KIND (INCLUDING WITHOUT LIMITATION PUNITIVE OR MULTIPLE DAMAGES OR OTHER
 SPECIAL, DIRECT, INDIRECT, EXEMPLARY, INCIDENTAL, LOSS OF PROFITS, REVENUE
 OR DATA, COST OF COVER OR CONSEQUENTIAL LOSSES OR DAMAGES OF ANY KIND),
 HOWEVER CAUSED, AND REGARDLESS OF THE THEORY OF LIABILITY, WHETHER DERIVED
 FROM CONTRACT, TORT (INCLUDING, BUT NOT LIMITED TO, NEGLIGENCE), OR OTHERWISE,
 ARISING OUT OF OR RELATING TO THE DATA OR ITS USE OR ANY OTHER PERFORMANCE,
 WHETHER OR NOT AUTODESK HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS
 OR DAMAGE. 

**************************************************************************************/

#include <fbxsdk.h>
#include <fbxfilesdk/kfbxio/kfbxstreamoptionsfbx.h>

// kfbxiosettings will replace stream options in the near future
#include <fbxfilesdk/kfbxio/kfbxiosettings.h>

#include "../Common/Common.h"

void InitializeSdkObjects(KFbxSdkManager*& pSdkManager, KFbxScene*& pScene)
{
    // The first thing to do is to create the FBX SDK manager which is the 
    // object allocator for almost all the classes in the SDK.
    pSdkManager = KFbxSdkManager::Create();

    if (!pSdkManager)
    {
        printf("Unable to create the FBX SDK manager\n");
        exit(0);
    }

    // Create the entity that will hold the scene.
    pScene = KFbxScene::Create(pSdkManager,"");

}

void DestroySdkObjects(KFbxSdkManager* pSdkManager)
{
    // Delete the FBX SDK manager. All the objects that have been allocated 
    // using the FBX SDK manager and that haven't been explicitely destroyed 
    // are automatically destroyed at the same time.
    if (pSdkManager) pSdkManager->Destroy();
    pSdkManager = NULL;
}

bool SaveScene(KFbxSdkManager* pSdkManager, KFbxDocument* pScene, const char* pFilename, int pFileFormat, bool pEmbedMedia)
{
    int lMajor, lMinor, lRevision;
    bool lStatus = true;

    // Create an exporter.
    KFbxExporter* lExporter = KFbxExporter::Create(pSdkManager, "");

    // Initialize the exporter by providing a filename.
    if(lExporter->Initialize(pFilename) == false)
    {
        printf("Call to KFbxExporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lExporter->GetLastErrorString());
        return false;
    }

    KFbxIO::GetCurrentVersion(lMajor, lMinor, lRevision);
    printf("FBX version number for this version of the FBX SDK is %d.%d.%d\n\n", lMajor, lMinor, lRevision);


    if( pFileFormat < 0 || pFileFormat >= pSdkManager->GetIOPluginRegistry()->GetWriterFormatCount() )
    {
        // Write in fall back format if pEmbedMedia is true
        pFileFormat = pSdkManager->GetIOPluginRegistry()->GetNativeWriterFormat();

        if (!pEmbedMedia)
        {
            //Try to export in ASCII if possible
            int lFormatIndex, lFormatCount = pSdkManager->GetIOPluginRegistry()->GetWriterFormatCount();

            for (lFormatIndex=0; lFormatIndex<lFormatCount; lFormatIndex++)
            {
                if (pSdkManager->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
                {
                    KString lDesc =pSdkManager->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
                    char *lASCII = "ascii";
                    if (lDesc.Find(lASCII)>=0)
                    {
                        pFileFormat = lFormatIndex;
                        break;
                    }
                }
            }
        }
    }

    // Set the file format
    lExporter->SetFileFormat(pFileFormat);

    KFbxStreamOptions* lExportOptions = NULL;
    if (pSdkManager->GetIOPluginRegistry()->WriterIsFBX(pFileFormat))
    {
        lExportOptions = KFbxStreamOptionsFbxWriter::Create(pSdkManager, "");

        // Set the export states. By default, the export states are always set to 
        // true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below 
        // shows how to change these states.
        lExportOptions->SetOption(KFBXSTREAMOPT_FBX_MATERIAL, true);
        lExportOptions->SetOption(KFBXSTREAMOPT_FBX_TEXTURE, true);
        lExportOptions->SetOption(KFBXSTREAMOPT_FBX_EMBEDDED, pEmbedMedia);
        lExportOptions->SetOption(KFBXSTREAMOPT_FBX_LINK, true);
        lExportOptions->SetOption(KFBXSTREAMOPT_FBX_SHAPE, true);
        lExportOptions->SetOption(KFBXSTREAMOPT_FBX_GOBO, true);
        lExportOptions->SetOption(KFBXSTREAMOPT_FBX_ANIMATION, true);
        lExportOptions->SetOption(KFBXSTREAMOPT_FBX_GLOBAL_SETTINGS, true);
    }




    // Export the scene.
    lStatus = lExporter->Export(pScene, lExportOptions); 

    if(lExportOptions)
        lExportOptions->Destroy();
    lExportOptions=NULL;
    // Destroy the exporter.
    lExporter->Destroy();
    return lStatus;
}

bool LoadScene(KFbxSdkManager* pSdkManager, KFbxDocument* pScene, const char* pFilename)
{
    int lFileMajor, lFileMinor, lFileRevision;
    int lSDKMajor,  lSDKMinor,  lSDKRevision;
    int lFileFormat = -1;
    int i, lTakeCount;
    KString lCurrentTakeName;
    bool lStatus;
    char lPassword[1024];

    KFbxStreamOptionsFbxReader* lImportOptions=KFbxStreamOptionsFbxReader::Create(pSdkManager, "");


    // Get the file version number generate by the FBX SDK.
    KFbxIO::GetCurrentVersion(lSDKMajor, lSDKMinor, lSDKRevision);

    // Create an importer.
    KFbxImporter* lImporter = KFbxImporter::Create(pSdkManager,"");

    if (!pSdkManager->GetIOPluginRegistry()->DetectFileFormat(pFilename, lFileFormat))
    {
        // Unrecognizable file format. Try to fall back to native format.
        lFileFormat = pSdkManager->GetIOPluginRegistry()->GetNativeReaderFormat();
    }
    lImporter->SetFileFormat(lFileFormat);

    // Initialize the importer by providing a filename.
    const bool lImportStatus = lImporter->Initialize(pFilename);
    lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

    if( !lImportStatus )
    {
        printf("Call to KFbxImporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lImporter->GetLastErrorString());

        if (lImporter->GetLastErrorID() == KFbxIO::eFILE_VERSION_NOT_SUPPORTED_YET ||
            lImporter->GetLastErrorID() == KFbxIO::eFILE_VERSION_NOT_SUPPORTED_ANYMORE)
        {
            printf("FBX version number for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
            printf("FBX version number for file %s is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
        }

        return false;
    }

    printf("FBX version number for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

    if (lImporter->IsFBX())
    {
        printf("FBX version number for file %s is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);

        // From this point, it is possible to access take information without
        // the expense of loading the entire file.

        printf("Take Information\n");

        lTakeCount = lImporter->GetTakeCount();

        printf("    Number of takes: %d\n", lTakeCount);
        printf("    Current take: \"%s\"\n", lImporter->GetCurrentTakeName());
        printf("\n");

        for(i = 0; i < lTakeCount; i++)
        {
            KFbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

            printf("    Take %d\n", i);
            printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
            printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

            // Change the value of the import name if the take should be imported 
            // under a different name.
            printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

            // Set the value of the import state to false if the take should be not
            // be imported. 
            printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
            printf("\n");
        }

        //IOSREF.SetBoolProp(IMP_FBX "|" "SOPT_FBX_MATERIAL",       true);
        //IOSREF.SetBoolProp(IMP_FBX "|" "SOPT_FBX_TEXTURE",        true);
        //IOSREF.SetBoolProp(IMP_FBX "|" "SOPT_FBX_LINK",           true);
        //IOSREF.SetBoolProp(IMP_FBX "|" "SOPT_FBX_SHAPE",          true);
        //IOSREF.SetBoolProp(IMP_FBX "|" "SOPT_FBX_GOBO",           true);
        //IOSREF.SetBoolProp(IMP_FBX "|" "SOPT_FBX_ANIMATION",      true);
        //IOSREF.SetBoolProp(IMP_FBX "|" "SOPT_FBX_GLOBAL_SETTINGS",true);


        // Set the import states. By default, the import states are always set to 
        // true. The code below shows how to change these states.
        lImportOptions->SetOption(KFBXSTREAMOPT_FBX_MATERIAL, true);
        lImportOptions->SetOption(KFBXSTREAMOPT_FBX_TEXTURE, true);
        lImportOptions->SetOption(KFBXSTREAMOPT_FBX_LINK, true);
        lImportOptions->SetOption(KFBXSTREAMOPT_FBX_SHAPE, true);
        lImportOptions->SetOption(KFBXSTREAMOPT_FBX_GOBO, true);
        lImportOptions->SetOption(KFBXSTREAMOPT_FBX_ANIMATION, true);
        lImportOptions->SetOption(KFBXSTREAMOPT_FBX_GLOBAL_SETTINGS, true);
    }

    // Import the scene.
    lStatus = lImporter->Import(pScene, lImportOptions);

    if(lStatus == false && lImporter->GetLastErrorID() == KFbxIO::ePASSWORD_ERROR)
    {
        printf("Please enter password: ");

        lPassword[0] = '\0';

        scanf("%s", lPassword);
        KString lString(lPassword);

        //IOSREF.SetStringProp(IMP_FBX "|" "SOPT_FBX_PASSWORD", lString);
        lImportOptions->SetOption(KFBXSTREAMOPT_FBX_PASSWORD, lString);

        //IOSREF.SetBoolProp(IMP_FBX "|" "SOPT_FBX_PASSWORD_ENABLE", true);
        lImportOptions->SetOption(KFBXSTREAMOPT_FBX_PASSWORD_ENABLE, true);

        lStatus = lImporter->Import(pScene, lImportOptions);

        if(lStatus == false && lImporter->GetLastErrorID() == KFbxIO::ePASSWORD_ERROR)
        {
            printf("\nPassword is wrong, import aborted.\n");
        }
    }

    // Destroy the importer.
    if(lImportOptions)
        lImportOptions->Destroy();
    lImportOptions=NULL;
    lImporter->Destroy();

    return lStatus;
}

