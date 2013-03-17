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
require_once("openmediavault/module.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");

class OMVLoopMgmtModule extends OMVModuleAbstract
  implements OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "loopmgmt";
	}

	/**
	 * eate the mdadm configuration file.
	 */
	public function loopCreate() {
//		$cmd = "sudo omv-mkconf mdadm 2>&1";
//		OMVUtil::exec($cmd, $output, $result);
//		if ($result !== 0) {
//			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
//			  $cmd, implode("\n", $output));
//		}
	}

	public function loopAttach() {
//		$cmd = "sudo omv-mkconf mdadm 2>&1";
//		OMVUtil::exec($cmd, $output, $result);
//		if ($result !== 0) {
//			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
//			  $cmd, implode("\n", $output));
//		}
	}

	public function loopDetach() {
//		$cmd = "sudo omv-mkconf mdadm 2>&1";
//		OMVUtil::exec($cmd, $output, $result);
//		if ($result !== 0) {
//			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
//			  $cmd, implode("\n", $output));
//		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.loop.device",
		  array($this, "loopCreate"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.loop.device",
		  array($this, "loopAttach"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.loop.device",
		  array($this, "loopDetach"));
	}
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVLoopMgmtModule());
?>
