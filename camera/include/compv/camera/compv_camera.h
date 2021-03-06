/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CAMERA_CAMERA_H_)
#define _COMPV_CAMERA_CAMERA_H_

#include "compv/camera/compv_camera_config.h"
#include "compv/camera/compv_camera_common.h"
#include "compv/base/compv_sharedlib.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mat.h"
#include "compv/base/compv_caps.h"

#include <vector>

COMPV_NAMESPACE_BEGIN()

//
//	Capabilities
//
enum {
	COMPV_CAMERA_CAP_INT_WIDTH,
	COMPV_CAMERA_CAP_INT_HEIGHT,
	COMPV_CAMERA_CAP_INT_FPS,
	COMPV_CAMERA_CAP_INT_SUBTYPE, // COMPV_SUBTYPE pixel format (e.g. COMPV_SUBTYPE_PIXELS_YUV420P)
	COMPV_CAMERA_CAP_BOOL_AUTOFOCUS,
};

//
//	CompVCameraDeviceInfo
//
struct CompVCameraDeviceInfo {
    std::string id;
    std::string name;
    std::string description;
    CompVCameraDeviceInfo(const std::string& id_, const std::string& name_, const std::string& description_) {
        id = id_;
        name = name_;
        description = description_;
    }
};
typedef std::vector<CompVCameraDeviceInfo > CompVCameraDeviceInfoList;


//
//	CompVCameraListener
//
COMPV_OBJECT_DECLARE_PTRS(CameraListener)

class COMPV_CAMERA_API CompVCameraListener : public CompVObj
{
protected:
	CompVCameraListener() { }
public:
	virtual ~CompVCameraListener() {  }
	COMPV_OBJECT_GET_ID(CompVCameraListener);

	virtual COMPV_ERROR_CODE onNewFrame(const CompVMatPtr& image) {
		return COMPV_ERROR_CODE_S_OK;
	}
	virtual COMPV_ERROR_CODE onError(const std::string& message) {
		return COMPV_ERROR_CODE_S_OK;
	}
};

//
//	CompVCamera
//
COMPV_OBJECT_DECLARE_PTRS(Camera)

typedef COMPV_ERROR_CODE(*CompVCameraNewFunc)(CompVCameraPtrPtr camera);

class COMPV_CAMERA_API CompVCamera : public CompVObj, public CompVCaps
{
protected:
    CompVCamera();
public:
    virtual ~CompVCamera();

    static COMPV_ERROR_CODE init();
    static COMPV_ERROR_CODE deInit();
    static COMPV_INLINE bool isInitialized() {
        return s_bInitialized;
    }

    virtual COMPV_ERROR_CODE devices(CompVCameraDeviceInfoList& list) = 0;
    virtual COMPV_ERROR_CODE start(const std::string& deviceId = "") = 0;
    virtual COMPV_ERROR_CODE stop() = 0;

	virtual COMPV_ERROR_CODE setListener(CompVCameraListenerPtr listener);

    static COMPV_ERROR_CODE newObj(CompVCameraPtrPtr camera);

    struct CameraFactory {
        CompVCameraNewFunc funcNew;
        CompVSharedLibPtr lib;
        bool isValid() {
            return funcNew && lib;
        }
        void deInit() {
            funcNew = NULL;
            lib = NULL; // calls the destructor -> will close the DLL and free the handle
        }
    };

protected:
	COMPV_INLINE CompVCameraListenerPtr listener()const {
		return m_ptrListener;
	}

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    static bool s_bInitialized;
    static CameraFactory s_CameraFactory;
	CompVCameraListenerPtr m_ptrListener;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CAMERA_CAMERA_H_ */
