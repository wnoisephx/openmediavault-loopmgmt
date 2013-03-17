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
require_once("openmediavault/globals.inc");
require_once("openmediavault/object.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/util.inc");

/**
 * Class to handle a physical volume.
 */
class OMVLoopDevice extends OMVObject {
	protected $deviceFile = "";
	protected $name = "";
	protected $size = "";
	protected $image = "";
	protected $serial = "";
	private $dataCached = FALSE;

	/**
	 * Constructor
	 * @param deviceFile The device file, e.g. /dev/sdb.
	 */
	public function __construct($deviceFile) {
		$this->deviceFile = $deviceFile;
	}

	/**
	 * Get the physical volume detailed informations.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if ($this->dataCached !== FALSE)
			return TRUE;

		// Parse command output:
		// /dev/loop0: [0801]:288503 (/mnt/tst.dd)
		$cmd = sprintf("export LANG=C; sudo losetup --show %s", $this->getDeviceFile());
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;

		$output = explode(" ", trim($output[0]));
		$tst = explode(":",$output[1]);
		$inode = $tst[1];
		$this->image = substr($output[2], 1, -1);

		/**
		 * loseup can't display long pathnames, so if the file check to see if it's been
		 * truncated.  If it has the last charater will be a *
		 */
		$tst=$this->image[strlen($this->image)-1];
		if($tst=="*") {
			/**
			 * it's been truncated - Finding the real image name is a pain,
			 * we have the inode number, so we are going to have to use the find
			 * command to search for it
			 */
			$cmd = sprintf("export LANG=C; sudo find %s -inum %d",$this->image,$inode);
	                @OMVUtil::exec($cmd, $out, $result);
       		        if ($result !== 0)
                        	return FALSE;
			$this->image=$out[0];
		}

		// get the name of the attached image and set it as the name/serialnumber
		$tst = explode("/",$this->image);
		$this->name = $tst[sizeof($tst)-1];
		$this->serial = md5($this->name);

		// get the size of the image that this device is pointing to
		unset($out);
		$cmd = sprintf("export LANG=C; sudo du -sb %s",$this->image);
	        @OMVUtil::exec($cmd, $out, $result);
       		if ($result !== 0)
                       	return FALSE;
		$tst = explode("/",$out[0]);
		$this->size = $tst[0];

		// Set flag to mark informations has been successfully read.
		$this->dataCached = TRUE;

		return TRUE;
	}

	/**
	 * Refresh the cached informations.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		if ($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Checks if the loop device exists.
	 * @return TRUE if the loop device exists, otherwise FALSE.
	 */
	public function exists() {
		if ($this->getData() === FALSE)
			return FALSE;
		return !empty($this->image);
	}

	/**
	 * Get the device path, e.g. /dev/loop0, /dev/loop2.
	 * @return The device path.
	 */
	public function getDeviceFile() {
		return $this->deviceFile;
	}

	/**
	 * Get the size of the loop device in bytes.
	 * this really isn't the size of the device, but rather the size of the image it's
	 * pointing to.
	 * @return The size of the loop device in bytes as string,
	 * FALSE on failure.
	 */
	public function getSize() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->size;
	}

	/**
	 * Get the location of the image the loop device is pointing to.
	 * @return The image location or FALSE on failure.
	 */
	public function getImage() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->image;
	}

        /**
         * Get the imagename sans any path info
         * @param deviceFile  The device file, e.g. /dev/loop0
         * @return imagename
         */
         public function getName() {
		if($this->getData() === FALSE)
			return FALSE;
                 return $this->name;
	}

	public function getSerial() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->serial;
	}
}

class OMVLoopDevices extends OMVObject {
        /**
         * Get a list of Loop Devices.
         * @return A list of Loop Devices or FALSE on failure.
         * Example: array(
         *   0 => /dev/loop0
         *   1 => /dev/loop6
         * )
         */
        public static function get() {
                $cmd = "export LANG=C; sudo losetup --all 2>/dev/null";
                @OMVUtil::exec($cmd, $output, $result);
                if ($result !== 0) {
                        return FALSE;
                }
                $list = array();
                foreach ($output as $outputk => $outputv) {
			$lp = explode(":", trim($outputv));
                        $list[] = trim($lp[0]);
                }
                return $list;
        }
}
?>
