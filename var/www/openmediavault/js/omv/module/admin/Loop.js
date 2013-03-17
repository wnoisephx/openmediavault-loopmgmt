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
// require("js/omv/NavigationPanel.js")
// require("js/omv/MessageBox.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/data/Store.js")
// require("js/omv/grid/TBarGridPanel.js")
// require("js/omv/FormPanelDialog.js")
// require("js/omv/form/CheckboxGrid.js")
// require("js/omv/form/plugins/FieldInfo.js")
// require("js/omv/util/Format.js")

Ext.ns("OMV.Module.Storage");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("storage", "loopmanagement", {
	text: _("Loop Device Management"),
	icon: "images/filesystem.png",
	position: 20
});

/**
 * @class OMV.Module.Storage.RAIDGridPanel
 * @derived OMV.grid.TBarGridPanel
 */
OMV.Module.Storage.LoopGridPanel = function(config) {
	var initialConfig = {
		autoReload: false,
		reloadInterval: 10000, // 10 seconds
		disableLoadMaskOnLoad: true,
		hideAdd: true,
		hideEdit: true,
		hideDelete: true,
		hideRefresh: false,
		hidePagingToolbar: false,
		addButtonText: "Create",
		stateId: "4ba1f909-9f57-4aab-87eb-638385c30f46",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: _("Image File"),
				sortable: true,
				dataIndex: "image",
				id: "image"
			},{
				header: _("Size"),
				sortable: true,
				dataIndex: "size",
				id: "size",
				width: 50,
				renderer: OMV.util.Format.binaryUnitRenderer()
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LoopGridPanel.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LoopGridPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy({
				"service": "LoopMgmt",
				"method": "enumerateLoopDevices"
			}),
			reader: new Ext.data.JsonReader({
				idProperty: "devicefile",
				totalProperty: "total",
//				root: "data",
				fields: [
					{ name: "devicefile" },
					{ name: "image" },
					{ name: "size" },
    			]
			})
		});
		OMV.Module.Storage.LoopGridPanel.superclass.initComponent.
		  apply(this, arguments);
	},

	initToolbar : function() {
		var tbar = OMV.Module.Storage.LoopGridPanel.superclass.initToolbar.
		  apply(this);
		// Add 'Create' button to top toolbar
		tbar.insert(1, {
			id: this.getId() + "-create",
			xtype: "button",
			text: _("Create"),
			icon: "images/filesystem.png",
			handler: this.cbCreateBtnHdl,
			scope: this,
			disabled: false
		});
		// Add 'Attach' button to top toolbar
		tbar.insert(2, {
			id: this.getId() + "-attach",
			xtype: "button",
			text: _("Attach"),
			icon: "images/filesystem.png",
			handler: this.cbAttachBtnHdl,
			scope: this,
			disabled: false
		});
		// Add 'Detach' button to top toolbar
		tbar.insert(3, {
			id: this.getId() + "-detach",
			xtype: "button",
			text: _("Detach"),
			icon: "images/delete.png",
			handler: this.cbDetachBtnHdl,
			scope: this,
			disabled: true
		});
		return tbar;
	},

	cbSelectionChangeHdl : function(model) {
		OMV.Module.Storage.LoopGridPanel.superclass.cbSelectionChangeHdl.
		  apply(this, arguments);

                // Process additional buttons
                var tbarBtnName = [ "detach" ];
                var tbarBtnDisabled = {
                        "detach": true
                };
                var records = model.getSelections();
                // Enable/disable buttons depending on the number of selected rows.
                if (records.length == 1) {
                        tbarBtnDisabled["detach"] = false;
                }

		for (var i = 0; i < tbarBtnName.length; i++) {
			var tbarBtnCtrl = this.getTopToolbar().findById(this.getId() +
			  "-" + tbarBtnName[i]);
			if (!Ext.isEmpty(tbarBtnCtrl)) {
				if (true == tbarBtnDisabled[tbarBtnName[i]]) {
					tbarBtnCtrl.disable();
				} else {
					tbarBtnCtrl.enable();
				}
			}
		}
	},

	cbAttachBtnHdl : function() {
		var wnd = new OMV.Module.Storage.LoopAttachDialog({
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

        cbCreateBtnHdl : function() {
                var wnd = new OMV.Module.Storage.LoopCreateDialog({
                        listeners: {
                                submit: function() {
                                        this.doReload();
                                },
                                scope: this
                        }
                });
                wnd.show();
        },

	cbDetachBtnHdl : function() {
                var selModel = this.getSelectionModel();
                var record = selModel.getSelected();
                OMV.MessageBox.show({
                        title: _("Confirmation"),
                        msg: _("Do you really want to detach the selected image?"),
                        buttons: Ext.Msg.YESNO,
                        fn: function(answer) {
                                if (answer == "no")
                                        return;

                                // Execute RPC
		                OMV.Ajax.request(this.cbLoopCallBack, this, "loopmgmt", "detach",
                  			{ "devicefile": record.get("devicefile") });
			},
                        scope: this,
                        icon: Ext.Msg.QUESTION
                });
	},
	
	cbLoopCallBack : function() {
			this.doReload();
	}
});
OMV.NavigationPanelMgr.registerPanel("storage", "loopmanagement", {
	cls: OMV.Module.Storage.LoopGridPanel
});

/**
 * @class OMV.Module.Storage.LoopAttachDialog
 * @derived OMV.FormPanelDialog
 */
OMV.Module.Storage.LoopAttachDialog = function(config) {
	var initialConfig = {
		rpcService: "LoopMgmt",
		rpcSetMethod: "attach",
		title: _("Attach Loop device"),
		autoHeight: true,
		hideReset: true,
		width: 550
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.LoopAttachDialog.superclass.constructor.call(
	  this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LoopAttachDialog, OMV.FormPanelDialog, {
	getFormConfig : function() {
		return {
			autoHeight: true
		};
	},

	getFormItems : function() {
		return [{
			xtype: "textfield",
			name: "image",
			fieldLabel: _("Image Location"),
			vtype: "noBlank"
		}];
	},

	doSubmit : function() {
		OMV.Module.Storage.LoopAttachDialog.superclass.doSubmit.apply(this, arguments);
	}
});

/**
 * @class OMV.Module.Storage.LoopCreateDialog
 * @derived OMV.FormPanelDialog
 */
OMV.Module.Storage.LoopCreateDialog = function(config) {
        var initialConfig = {
                rpcService: "LoopMgmt",
                rpcSetMethod: "create",
                title: _("Create Loop device"),
                autoHeight: true,
                hideReset: true,
                width: 550
        };
        Ext.apply(initialConfig, config);
        OMV.Module.Storage.LoopAttachDialog.superclass.constructor.call(
          this, initialConfig);
};
Ext.extend(OMV.Module.Storage.LoopCreateDialog, OMV.FormPanelDialog, {
        getFormConfig : function() {
                return {
                        autoHeight: true
                };
        },

        getFormItems : function() {
                return [{
                        	xtype: "textfield",
                      	  	name: "image",
                        	fieldLabel: _("Image Location"),
                        	vtype: "noBlank" 
			},{	
	                        xtype: "compositefield",
        	                fieldLabel: _("Size"),
                	        combineErrors: false,
                        	items: [{
	                                xtype: "hidden",
       		                        name: "sizebox"
       		                },{
                	                xtype: "textfield",
                        	        name: "size",
	                        },{
			        	xtype: "combo",
	                        	name: "sizebytes",
        	                	hiddenName: "sizebytes",
                        		mode: "local",
                        		store: [
	                                	[ "KB","KB" ],
        	                        	[ "MB","MB" ],
                	                	[ "GB","GB" ],
                        	        	[ "TB","TB" ]
                        		],
	                        	allowBlank: false,
        	                	editable: false,
                	        	triggerAction: "all",
        	                }]
                }];
        },

        doSubmit : function() {
                OMV.Module.Storage.LoopCreateDialog.superclass.doSubmit.apply(this, arguments);
        }
});

