<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    William Ritchie <billr@domsys.com>
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 *
 * This file was derived in part from filesystemmgmt.inc
 * All orignal parts are copyrighted by their respective owners
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
 */
require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/system.inc");
require_once("openmediavault/loop.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class LoopMgmtRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"enumerateLoopDevices" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"create" => array('{
				"type":"object",
				"properties":{
					"devices":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE_LIST'].'},
					"image":{"type":"string"},
					"size":{"type":"number"),
					"sizebytes":{"type":"string"),
				}
			}'),
			"attach" => array('{
				"type":"object",
				"properties":{
					"image":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'}
				}
			}'),
			"detach" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'}
				}
			}'),
		);
	}

	/**
	 * Enumerate all Loop devices on the system.
	 * @return An array containing Loop device objects with following fields:
	 *   name, devicefile, uuid, size, description.
	 */
        public function enumerateLoopDevices() {
                // Check permissions
                $this->validateSession();
                if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
                        throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
                }
                $this->commitSession();
                // Enumerate all loop decices on the system.
                if (FALSE === ($lpds = OMVLoopDevices::get())) {
                        throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
                          "get list of Loop Devices");
                }
                $result = array();
                foreach ($lpds as $lpdk => $lpv) {
                        // Get the loop device details.
                        $lp = new OMVLoopDevice($lpv);
                        $result[] = array(
                                "devicefile" => $lp->getDeviceFile(),
                                "image" => $lp->getImage(),
                                "size" => $lp->getSize(),
                        );
                }
                return $result;
	}

	/**
	 * Create an image file of size lenght and attach it to a loop device.
	 */
	public function create($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
//		$this->validateParams(__METHOD__, func_get_args());

		// create the image file
                $cmd = sprintf("export LANG=C; sudo dd if=/dev/zero of=%s bs=1 count=0 seek=%d%s",$data['image'],$data['size'],$data['sizebytes']);
                @OMVUtil::exec($cmd, $output, $result);
                if ($result !== 0)
                        return FALSE;

                // try and attach the image
                $cmd = sprintf("export LANG=C; sudo losetup --find %s", $data['image']);
                @OMVUtil::exec($cmd, $output, $result);
                if ($result !== 0)
                        return FALSE;

		// Notify configuration change
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.loop.device", $data);
	}

	/**
	/**
	 * Attach an image to a loop device.
	 */
	public function attach($data) {
		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
//		$this->validateParams(__METHOD__, func_get_args());

		// does the image file exist?
		if(!file_exists($data['image'])) {
			 throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_FILE_NOT_FOUND,$data['image']);
		}

		// try and attach the image
                $cmd = sprintf("export LANG=C; sudo losetup --find %s", $data['image']);
                @OMVUtil::exec($cmd, $output, $result);
                if ($result !== 0)
                        return FALSE;

		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.loop.device", $data);
	}

	/**
	 * Detach an image from a loop device.
	 */
	public function detach($data) {
		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
//		$this->validateParams(__METHOD__, func_get_args());

		// try and detach the image
                $cmd = sprintf("export LANG=C; sudo losetup --detach %s", $data['devicefile']);
                @OMVUtil::exec($cmd, $output, $result);
                if ($result !== 0)
                        return FALSE;

		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.loop.device", $data);
	}
}
?>
