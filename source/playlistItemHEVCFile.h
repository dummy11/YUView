/*  YUView - YUV player with advanced analytics toolset
*   Copyright (C) 2015  Institut für Nachrichtentechnik
*                       RWTH Aachen University, GERMANY
*
*   YUView is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   YUView is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with YUView.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PLAYLISTITEMHEVCFILE_H
#define PLAYLISTITEMHEVCFILE_H

#include "playlistItemIndexed.h"
#include "videoHandlerYUV.h"
#include "fileSourceHEVCAnnexBFile.h"
#include "de265.h"
#include "statisticHandler.h"

#include <QLibrary>

class videoHandler;

class playlistItemHEVCFile :
  public playlistItemIndexed
{
  Q_OBJECT

public:

  /* The default constructor requires the user to set a name that will be displayed in the treeWidget and
   * provide a pointer to the widget stack for the properties panels. The constructor will then call
   * addPropertiesWidget to add the custom properties panel.
  */
  playlistItemHEVCFile(QString fileName);
  virtual ~playlistItemHEVCFile();

  // Save the HEVC file element to the given xml structure.
  virtual void savePlaylist(QDomElement &root, QDir playlistDir) Q_DECL_OVERRIDE;
  // Create a new playlistItemHEVCFile from the playlist file entry. Return NULL if parsing failed.
  static playlistItemHEVCFile *newplaylistItemHEVCFile(QDomElementYUV root, QString playlistFilePath);

  // Return the info title and info list to be shown in the fileInfo groupBox.
  // The default implementations will return empty strings/list.
  virtual QString getInfoTitel() Q_DECL_OVERRIDE { return "HEVC File Info"; }
  virtual QList<infoItem> getInfoList() Q_DECL_OVERRIDE;

  virtual QString getPropertiesTitle() Q_DECL_OVERRIDE { return "HEVC File Properties"; };

  virtual QSize  getSize()      Q_DECL_OVERRIDE { return yuvVideo.getFrameSize(); }
  
  // Draw the item using the given painter and zoom factor. If the item is indexed by frame, the given frame index will be drawn. If the
  // item is not indexed by frame, the parameter frameIdx is ignored.
  virtual void drawItem(QPainter *painter, int frameIdx, double zoomFactor) Q_DECL_OVERRIDE;

  // Return the source (YUV and statistics) values under the given pixel position.
  virtual ValuePairListSets getPixelValues(QPoint pixelPos) Q_DECL_OVERRIDE;

  // If you want your item to be droppable onto a difference object, return true here and return a valid video handler.
  virtual bool canBeUsedInDifference() Q_DECL_OVERRIDE { return true; }
  virtual videoHandler *getVideoHandler() Q_DECL_OVERRIDE { return &yuvVideo; }

  // Override from playlistItemIndexed. The annexBFile handler can tell us how many POSs there are.
  virtual indexRange getstartEndFrameLimits() Q_DECL_OVERRIDE { return indexRange(0, annexBFile.getNumberPOCs()-1); }

public slots:
  // Load the YUV data for the given frame index from file. This slot is called by the videoHandlerYUV if the frame that is
  // requested to be drawn has not been loaded yet.
  virtual void loadYUVData(int frameIdx);

  // The videoHandlerYUV want's to know if the current frame range changed.
  virtual void slotUpdateFrameRange();

  // The statistic with the given frameIdx/typeIdx could not be found in the cache. Load it.
  virtual void loadStatisticToCache(int frameIdx, int typeIdx);

protected:
  virtual void createPropertiesWidget() Q_DECL_OVERRIDE;

private:

  // The Annex B source file
  fileSourceHEVCAnnexBFile annexBFile;

  videoHandlerYUV yuvVideo;

  void setDe265ChromaMode(const de265_image *img);

  // ------------ Everything we need to plug into the libde265 library ------------

  de265_decoder_context* p_decoder;

  // typedefs for libde265 decoder library function pointers
  typedef de265_decoder_context *(*f_de265_new_decoder)          ();
  typedef void                   (*f_de265_set_parameter_bool)   (de265_decoder_context*, de265_param, int);
  typedef void                   (*f_de265_set_parameter_int)    (de265_decoder_context*, de265_param, int);
  typedef void                   (*f_de265_disable_logging)      ();
  typedef void                   (*f_de265_set_verbosity)        (int);
  typedef de265_error            (*f_de265_start_worker_threads) (de265_decoder_context*, int);
  typedef void                   (*f_de265_set_limit_TID)        (de265_decoder_context*, int);
  typedef const char*            (*f_de265_get_error_text)       (de265_error);
  typedef de265_chroma           (*f_de265_get_chroma_format)    (const de265_image*);
  typedef int                    (*f_de265_get_image_width)      (const de265_image*, int);
  typedef int                    (*f_de265_get_image_height)     (const de265_image*, int);
  typedef const uint8_t*         (*f_de265_get_image_plane)      (const de265_image*, int, int*);
  typedef int                    (*f_de265_get_bits_per_pixel)   (const de265_image*, int);
  typedef de265_error            (*f_de265_decode)               (de265_decoder_context*, int*);
  typedef de265_error            (*f_de265_push_data)            (de265_decoder_context*, const void*, int, de265_PTS, void*);
  typedef de265_error            (*f_de265_flush_data)           (de265_decoder_context*);
  typedef const de265_image*     (*f_de265_get_next_picture)     (de265_decoder_context*);
  typedef de265_error            (*f_de265_free_decoder)         (de265_decoder_context*);

  // libde265 decoder library function pointers for internals
  typedef void (*f_de265_internals_get_CTB_Info_Layout)		   (const de265_image*, int*, int*, int*);
  typedef void (*f_de265_internals_get_CTB_sliceIdx)			   (const de265_image*, uint16_t*);
  typedef void (*f_de265_internals_get_CB_Info_Layout)		   (const de265_image*, int*, int*, int*);
  typedef void (*f_de265_internals_get_CB_info)				       (const de265_image*, uint16_t*);
  typedef void (*f_de265_internals_get_PB_Info_layout)		   (const de265_image*, int*, int*, int*);
  typedef void (*f_de265_internals_get_PB_info)				       (const de265_image*, int16_t*, int16_t*, int16_t*, int16_t*, int16_t*, int16_t*);
  typedef void (*f_de265_internals_get_IntraDir_Info_layout) (const de265_image*, int*, int*, int*);
  typedef void (*f_de265_internals_get_intraDir_info)			   (const de265_image*, uint8_t*, uint8_t*);
  typedef void (*f_de265_internals_get_TUInfo_Info_layout)	 (const de265_image*, int*, int*, int*);
  typedef void (*f_de265_internals_get_TUInfo_info)			     (const de265_image*, uint8_t*);

  // Decoder library
  void loadDecoderLibrary();
  void allocateNewDecoder();
  QLibrary p_decLib;

  // Status reporting
  QString p_StatusText;
  bool p_internalError;		///< There was an internal error and the decoder can not be used.

  // Decoder library function pointers
  f_de265_new_decoder			     de265_new_decoder;
  f_de265_set_parameter_bool   de265_set_parameter_bool;
  f_de265_set_parameter_int	   de265_set_parameter_int;
  f_de265_disable_logging		   de265_disable_logging;
  f_de265_set_verbosity		     de265_set_verbosity;
  f_de265_start_worker_threads de265_start_worker_threads;
  f_de265_set_limit_TID		     de265_set_limit_TID;
  f_de265_get_error_text		   de265_get_error_text;
  f_de265_get_chroma_format    de265_get_chroma_format;
  f_de265_get_image_width      de265_get_image_width;
  f_de265_get_image_height	   de265_get_image_height;
  f_de265_get_image_plane   	 de265_get_image_plane;
  f_de265_get_bits_per_pixel	 de265_get_bits_per_pixel;
  f_de265_decode 				       de265_decode;
  f_de265_push_data			       de265_push_data;
  f_de265_flush_data			     de265_flush_data;
  f_de265_get_next_picture	   de265_get_next_picture;
  f_de265_free_decoder    	   de265_free_decoder;

  // Decoder library function pointers for internals
  f_de265_internals_get_CTB_Info_Layout		    de265_internals_get_CTB_Info_Layout;
  f_de265_internals_get_CTB_sliceIdx			    de265_internals_get_CTB_sliceIdx;
  f_de265_internals_get_CB_Info_Layout		    de265_internals_get_CB_Info_Layout;
  f_de265_internals_get_CB_info				        de265_internals_get_CB_info;
  f_de265_internals_get_PB_Info_layout		    de265_internals_get_PB_Info_layout;
  f_de265_internals_get_PB_info				        de265_internals_get_PB_info;
  f_de265_internals_get_IntraDir_Info_layout  de265_internals_get_IntraDir_Info_layout;
  f_de265_internals_get_intraDir_info         de265_internals_get_intraDir_info;
  f_de265_internals_get_TUInfo_Info_layout	  de265_internals_get_TUInfo_Info_layout;
  f_de265_internals_get_TUInfo_info           de265_internals_get_TUInfo_info;

  // Was there an error? If everything is allright it will be DE265_OK.
  de265_error p_decError;

  /// ===== Buffering
#if SSE_CONVERSION
  byteArrayAligned  p_Buf_CurrentOutputBuffer;			      ///< The buffer that was requested in the last call to getOneFrame
#else
  QByteArray  p_Buf_CurrentOutputBuffer;			      ///< The buffer that was requested in the last call to getOneFrame
#endif
  int         p_Buf_CurrentOutputBufferFrameIndex;	///< The frame index of the buffer in p_Buf_CurrentOutputBuffer

  // Decode the next picture into the buffer. Return true on success.
#if SSE_CONVERSION
  bool decodeOnePicture(byteArrayAligned &buffer);
#else
  bool decodeOnePicture(QByteArray &buffer);
#endif
  // Copy the raw data from the de265_image src to the byte array
#if SSE_CONVERSION
  void copyImgToByteArray(const de265_image *src, byteArrayAligned &dst);
#else
  void copyImgToByteArray(const de265_image *src, QByteArray &dst);
#endif

  // --------------- Statistics ----------------

  // The statistics source
  statisticHandler statSource;

  // fill the list of statistic types that we can provide
  void fillStatisticList();
  // Get the statistics from the frame and put them into the cache
  void cacheStatistics(const de265_image *img, int iPOC);
  // With the given partitioning mode, the size of the CU and the prediction block index, calculate the
  // sub-position and size of the prediction block
  void getPBSubPosition(int partMode, int CUSizePix, int pbIdx, int *pbX, int *pbY, int *pbW, int *pbH);
  //
  void cacheStatistics_TUTree_recursive(uint8_t *tuInfo, int tuInfoWidth, int tuUnitSizePix, int iPOC, int tuIdx, int log2TUSize, int trDepth);

  // if set to true the decoder will also get statistics from each decoded frame and put them into the cache
  bool p_RetrieveStatistics;
  bool p_internalsSupported;		///< does the loaded library support the extraction of internals/statistics?

  // Convert intra direction mode into vector
  static const int p_vectorTable[35][2];

};

#endif // PLAYLISTITEMHEVCFILE_H