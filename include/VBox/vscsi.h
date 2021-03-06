/* $Id: vscsi.h $ */
/** @file
 * VBox storage drivers: Virtual SCSI driver
 */

/*
 * Copyright (C) 2006-2012 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 *
 * The contents of this file may alternatively be used under the terms
 * of the Common Development and Distribution License Version 1.0
 * (CDDL) only, as it comes in the "COPYING.CDDL" file of the
 * VirtualBox OSE distribution, in which case the provisions of the
 * CDDL are applicable instead of those of the GPL.
 *
 * You may elect to license modified versions of this file under the
 * terms and conditions of either the GPL or the CDDL or both.
 */

#ifndef ___VBox_vscsi_h
#define ___VBox_vscsi_h

#include <VBox/cdefs.h>
#include <VBox/types.h>
#include <iprt/sg.h>

RT_C_DECLS_BEGIN

#ifdef IN_RING0
# error "There are no VBox VSCSI APIs available in Ring-0 Host Context!"
#endif

/** A virtual SCSI device handle */
typedef struct VSCSIDEVICEINT *VSCSIDEVICE;
/** A pointer to a virtual SCSI device handle. */
typedef VSCSIDEVICE           *PVSCSIDEVICE;
/** A virtual SCSI LUN handle. */
typedef struct VSCSILUNINT    *VSCSILUN;
/** A pointer to a virtual SCSI LUN handle. */
typedef VSCSILUN              *PVSCSILUN;
/** A virtual SCSI request handle. */
typedef struct VSCSIREQINT    *VSCSIREQ;
/** A pointer to a virtual SCSI request handle. */
typedef VSCSIREQ              *PVSCSIREQ;
/** A SCSI I/O request handle. */
typedef struct VSCSIIOREQINT  *VSCSIIOREQ;
/** A pointer to a SCSI I/O request handle. */
typedef VSCSIIOREQ            *PVSCSIIOREQ;

/**
 * Virtual SCSI I/O request transfer direction.
 */
typedef enum VSCSIIOREQTXDIR
{
    /** Invalid direction */
    VSCSIIOREQTXDIR_INVALID = 0,
    /** Read */
    VSCSIIOREQTXDIR_READ,
    /** Write */
    VSCSIIOREQTXDIR_WRITE,
    /** Flush */
    VSCSIIOREQTXDIR_FLUSH,
    /** Unmap */
    VSCSIIOREQTXDIR_UNMAP,
    /** 32bit hack */
    VSCSIIOREQTXDIR_32BIT_HACK = 0x7fffffff
} VSCSIIOREQTXDIR;
/** Pointer to a SCSI LUN type */
typedef VSCSIIOREQTXDIR *PVSCSIIOREQTXDIR;

/**
 * LUN types we support
 */
typedef enum VSCSILUNTYPE
{
    /** Invalid type */
    VSCSILUNTYPE_INVALID = 0,
    /** Hard disk (SBC) */
    VSCSILUNTYPE_SBC,
    /** CD/DVD drive (MMC) */
    VSCSILUNTYPE_MMC,
    /** Last value to indicate an invalid device */
    VSCSILUNTYPE_LAST,
    /** 32bit hack */
    VSCSILUNTYPE_32BIT_HACK = 0x7fffffff
} VSCSILUNTYPE;
/** Pointer to a SCSI LUN type */
typedef VSCSILUNTYPE *PVSCSILUNTYPE;

/** The LUN can handle the UNMAP command. */
#define VSCSI_LUN_FEATURE_UNMAP          RT_BIT(0)
/** The LUN has a non rotational medium. */
#define VSCSI_LUN_FEATURE_NON_ROTATIONAL RT_BIT(1)
/** The medium of the LUN is readonly. */
#define VSCSI_LUN_FEATURE_READONLY       RT_BIT(2)

/**
 * Virtual SCSI LUN I/O Callback table.
 */
typedef struct VSCSILUNIOCALLBACKS
{
    /**
     * Retrieve the size of the underlying medium.
     *
     * @returns VBox status status code.
     * @param   hVScsiLun        Virtual SCSI LUN handle.
     * @param   pvScsiLunUser    Opaque user data which may
     *                           be used to identify the medium.
     * @param   pcbSize          Where to store the size of the
     *                           medium.
     */
    DECLR3CALLBACKMEMBER(int, pfnVScsiLunMediumGetSize, (VSCSILUN hVScsiLun,
                                                         void *pvScsiLunUser,
                                                         uint64_t *pcbSize));

    /**
     * Retrieve the sector size of the underlying medium.
     *
     * @returns VBox status status code.
     * @param   hVScsiLun        Virtual SCSI LUN handle.
     * @param   pvScsiLunUser    Opaque user data which may
     *                           be used to identify the medium.
     * @param   pcbSectorSize    Where to store the sector size of the
     *                           medium.
     */
    DECLR3CALLBACKMEMBER(int, pfnVScsiLunMediumGetSectorSize, (VSCSILUN hVScsiLun,
                                                              void *pvScsiLunUser,
                                                              uint32_t *pcbSectorSize));

    /**
     * Set the lock state of the underlying medium.
     *
     * @returns VBox status status code.
     * @param   hVScsiLun        Virtual SCSI LUN handle.
     * @param   pvScsiLunUser    Opaque user data which may
     *                           be used to identify the medium.
     * @param   fLocked          New lock state (locked/unlocked).
     */
    DECLR3CALLBACKMEMBER(int, pfnVScsiLunMediumSetLock, (VSCSILUN hVScsiLun,
                                                         void *pvScsiLunUser,
                                                         bool fLocked));
    /**
     * Enqueue a read or write request from the medium.
     *
     * @returns VBox status status code.
     * @param   hVScsiLun             Virtual SCSI LUN handle.
     * @param   pvScsiLunUser         Opaque user data which may
     *                                be used to identify the medium.
     * @param   hVScsiIoReq           Virtual SCSI I/O request handle.
     */
    DECLR3CALLBACKMEMBER(int, pfnVScsiLunReqTransferEnqueue, (VSCSILUN hVScsiLun,
                                                              void *pvScsiLunUser,
                                                              VSCSIIOREQ hVScsiIoReq));

    /**
     * Returns flags of supported features.
     *
     * @returns VBox status status code.
     * @param   hVScsiLun             Virtual SCSI LUN handle.
     * @param   pvScsiLunUser         Opaque user data which may
     *                                be used to identify the medium.
     * @param   hVScsiIoReq           Virtual SCSI I/O request handle.
     */
    DECLR3CALLBACKMEMBER(int, pfnVScsiLunGetFeatureFlags, (VSCSILUN hVScsiLun,
                                                           void *pvScsiLunUser,
                                                           uint64_t *pfFeatures));


} VSCSILUNIOCALLBACKS;
/** Pointer to a virtual SCSI LUN I/O callback table. */
typedef VSCSILUNIOCALLBACKS *PVSCSILUNIOCALLBACKS;

/**
 * The virtual SCSI request completed callback.
 */
typedef DECLCALLBACK(void) FNVSCSIREQCOMPLETED(VSCSIDEVICE hVScsiDevice,
                                               void *pvVScsiDeviceUser,
                                               void *pvVScsiReqUser,
                                               int rcScsiCode,
                                               bool fRedoPossible,
                                               int rcReq);
/** Pointer to a virtual SCSI request completed callback. */
typedef FNVSCSIREQCOMPLETED *PFNVSCSIREQCOMPLETED;

/**
 * Create a new empty SCSI device instance.
 *
 * @returns VBox status code.
 * @param   phVScsiDevice           Where to store the SCSI device handle.
 * @param   pfnVScsiReqCompleted    The method call after a request completed.
 * @param   pvVScsiDeviceUser       Opaque user data given in the completion callback.
 */
VBOXDDU_DECL(int) VSCSIDeviceCreate(PVSCSIDEVICE phVScsiDevice,
                                    PFNVSCSIREQCOMPLETED pfnVScsiReqCompleted,
                                    void *pvVScsiDeviceUser);

/**
 * Destroy a SCSI device instance.
 *
 * @returns VBox status code.
 * @param   hScsiDevice    The SCSI device handle to destroy.
 */
VBOXDDU_DECL(int) VSCSIDeviceDestroy(VSCSIDEVICE hVScsiDevice);

/**
 * Attach a LUN to the SCSI device.
 *
 * @returns VBox status code.
 * @param   hScsiDevice    The SCSI device handle to add the LUN to.
 * @param   hScsiLun       The LUN handle to add.
 * @param   iLun           The LUN number.
 */
VBOXDDU_DECL(int) VSCSIDeviceLunAttach(VSCSIDEVICE hVScsiDevice, VSCSILUN hVScsiLun, uint32_t iLun);

/**
 * Detach a LUN from the SCSI device.
 *
 * @returns VBox status code.
 * @param   hVScsiDevice    The SCSI device handle to add the LUN to.
 * @param   iLun            The LUN number to remove.
 * @param   phVScsiLun      Where to store the detached LUN handle.
 */
VBOXDDU_DECL(int) VSCSIDeviceLunDetach(VSCSIDEVICE hVScsiDevice, uint32_t iLun,
                                       PVSCSILUN phVScsiLun);

/**
 * Return the SCSI LUN handle.
 *
 * @returns VBox status code.
 * @param   hVScsiDevice    The SCSI device handle.
 * @param   iLun            The LUN number to get.
 * @param   phVScsiLun      Where to store the LUN handle.
 */
VBOXDDU_DECL(int) VSCSIDeviceLunGet(VSCSIDEVICE hVScsiDevice, uint32_t iLun,
                                    PVSCSILUN phVScsiLun);

/**
 * Enqueue a request to the SCSI device.
 *
 * @returns VBox status code.
 * @param   hVScsiDevice    The SCSI device handle.
 * @param   hVScsiReq       The SCSI request handle to enqueue.
 */
VBOXDDU_DECL(int) VSCSIDeviceReqEnqueue(VSCSIDEVICE hVScsiDevice, VSCSIREQ hVScsiReq);

/**
 * Allocate a new request handle.
 *
 * @returns VBox status code.
 * @param   phVScsiDevice     The SCSI device handle.
 * @param   phVScsiReq        Where to SCSI request handle.
 * @param   iLun              The LUN the request is for.
 * @param   pbCDB             The CDB for the request.
 * @param   cbCDB             The size of the CDB in bytes.
 * @param   cbSGList          Number of bytes the S/G list describes.
 * @param   cSGListEntries    Number of S/G list entries.
 * @param   paSGList          Pointer to the S/G list.
 * @param   pbSense           Pointer to the sense buffer.
 * @param   cbSense           Size of the sense buffer.
 * @param   pvVScsiReqUser    Opqaue user data returned when the request completes.
 */
VBOXDDU_DECL(int) VSCSIDeviceReqCreate(VSCSIDEVICE hVScsiDevice, PVSCSIREQ phVScsiReq,
                                       uint32_t iLun, uint8_t *pbCDB, size_t cbCDB,
                                       size_t cbSGList, unsigned cSGListEntries,
                                       PCRTSGSEG paSGList, uint8_t *pbSense,
                                       size_t cbSense, void *pvVScsiReqUser);

/**
 * Create a new LUN.
 *
 * @returns VBox status code.
 * @param   phVScsiLun              Where to store the SCSI LUN handle.
 * @param   enmLunType              The Lun type.
 * @param   pVScsiLunIoCallbacks    Pointer to the I/O callbacks to use for his LUN.
 * @param   pvVScsiLunUser          Opaque user argument which
 *                                  is returned in the pvScsiLunUser parameter
 *                                  when the request completion callback is called.
 */
VBOXDDU_DECL(int) VSCSILunCreate(PVSCSILUN phVScsiLun, VSCSILUNTYPE enmLunType,
                                 PVSCSILUNIOCALLBACKS pVScsiLunIoCallbacks,
                                 void *pvVScsiLunUser);

/**
 * Destroy virtual SCSI LUN.
 *
 * @returns VBox status code.
 * @param   hVScsiLun               The virtual SCSI LUN handle to destroy.
 */
VBOXDDU_DECL(int) VSCSILunDestroy(VSCSILUN hVScsiLun);

/**
 * Notify virtual SCSI LUN of medium being mounted.
 *
 * @returns VBox status code.
 * @param   hVScsiLun               The virtual SCSI LUN handle to destroy.
 */
VBOXDDU_DECL(int) VSCSILunMountNotify(VSCSILUN hVScsiLun);

/**
 * Notify virtual SCSI LUN of medium being unmounted.
 *
 * @returns VBox status code.
 * @param   hVScsiLun               The virtual SCSI LUN handle to destroy.
 */
VBOXDDU_DECL(int) VSCSILunUnmountNotify(VSCSILUN hVScsiLun);

/**
 * Notify a that a I/O request completed.
 *
 * @returns VBox status code.
 * @param   hVScsiIoReq             The I/O request handle that completed.
 *                                  This is given when a I/O callback for
 *                                  the LUN is called by the virtual SCSI layer.
 * @param   rcIoReq                 The status code the I/O request completed with.
 * @param   fRedoPossible           Flag whether it is possible to redo the request.
 *                                  If true setting any sense code will be omitted
 *                                  in case of an error to not alter the device state.
 */
VBOXDDU_DECL(int) VSCSIIoReqCompleted(VSCSIIOREQ hVScsiIoReq, int rcIoReq, bool fRedoPossible);

/**
 * Query the transfer direction of the I/O request.
 *
 * @returns Transfer direction.of the given I/O request
 * @param   hVScsiIoReq    The SCSI I/O request handle.
 */
VBOXDDU_DECL(VSCSIIOREQTXDIR) VSCSIIoReqTxDirGet(VSCSIIOREQ hVScsiIoReq);

/**
 * Query I/O parameters.
 *
 * @returns VBox status code.
 * @param   hVScsiIoReq    The SCSI I/O request handle.
 * @param   puOffset       Where to store the start offset.
 * @param   pcbTransfer    Where to store the amount of bytes to transfer.
 * @param   pcSeg          Where to store the number of segments in the S/G list.
 * @param   pcbSeg         Where to store the number of bytes the S/G list describes.
 * @param   ppaSeg         Where to store the pointer to the S/G list.
 */
VBOXDDU_DECL(int) VSCSIIoReqParamsGet(VSCSIIOREQ hVScsiIoReq, uint64_t *puOffset,
                                      size_t *pcbTransfer, unsigned *pcSeg,
                                      size_t *pcbSeg, PCRTSGSEG *ppaSeg);

/**
 * Query unmap parameters.
 *
 * @returns VBox status code.
 * @param   hVScsiIoReq    The SCSI I/O request handle.
 * @param   ppaRanges      Where to store the pointer to the range array on success.
 * @param   pcRanges       Where to store the number of ranges on success.
 */
VBOXDDU_DECL(int) VSCSIIoReqUnmapParamsGet(VSCSIIOREQ hVScsiIoReq, PCRTRANGE *ppaRanges,
                                           unsigned *pcRanges);

RT_C_DECLS_END

#endif /* ___VBox_vscsi_h */

