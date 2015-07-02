'use strict'

angular.module('myApp.controllers', []).
  controller('MenuBarCtrl', ['$scope', '$http', '$location', function ($scope, $http, $location) {
      // App-wide data
      $scope.new_br_name = null;
      $scope.bridges = null;
      // All tabs share this alert list
      $scope.alerts = [];
      $scope.year = new Date().getFullYear();

      // Common control methods
      $scope.closeAlert = function(index) {
          $scope.alerts.splice(index, 1);
      };


      (function init() {
          var promiseForBridges = $http.get('/bridges');
          promiseForBridges.then(function(response) {
              $scope.bridges = response.data.bridges;
          });
          var promiseForPhy = $http.get('/switchinfo');
          promiseForPhy.then(function(response) {
              $scope.switchInfo = response.data.switch_info;
          });
          var promiseForHelp = $http.get('/helptext');
          promiseForHelp.then(function(response) {
              $scope.helpText = response.data.help_text;
          });
      })();

      $scope.refreshSwitchInfo = function () {
          var promise =  $http.get('/switchinfo');
          promise.then(function(response) {
              $scope.switchInfo = response.data.switch_info;
          });
      };

      // Control methods

      $scope.add_br = function (brname) {
          var success = {type: "success", msg: "A bridge is being created. Please wait before it appears on the left panel."};
          $scope.alerts.push(success);
          $http({method: 'POST', url: '/bridges/add', params:{name:brname}}).
              success(function(data) {
                  if (data.ret === 0) {
                      $scope.bridges = data.bridges
                  } else {
                      var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                          $scope.alerts.push(failure);
                      }
                  });
      };

      $scope.del_br = function (br) {
          var success = {type: "success", msg: "A bridge is being deleted. Please wait before it disappears from the left panel."};
          $scope.alerts.push(success);
          $http({method: 'POST', url: '/bridges/' + br.name + '/del'}).
              success(function(data) {
                  if (data.ret === 0) {
                      $scope.bridges = data.bridges
                  } else {
                      var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                          $scope.alerts.push(failure);
                      }
                  });
      };

      // Common control methods shared between Configure page and Monitor page.
      $scope.commonMethods = {
          shortNames : function (members) {
            var simplifiedNames = [];
            angular.forEach(members, function (e) {
                this.push(e.split("/")[2]);
            }, simplifiedNames);
            return simplifiedNames.join(", ");
          },

          isLACP : function (type) {
              return (type === "lacp") ? true : false;
          },

          printFlags : function (flags) {
              var res = [];

              for (var property in flags) {
                  if (flags.hasOwnProperty(property)) {
                      if (flags[property]) {
                          res.push(property);
                      }
                  }
              }
              return res.join(" | ");
          },

          ofVersions : function (protocols) {
              if (typeof(protocols) === "object" && protocols) {
                  // array
                  return protocols.join();
              } else {
                  return protocols;
              }
          },

          isVlanConfigValid : function (port) {
              if (port) {
                  if (port.vlan_config.vlan_mode === "access" && port.vlan_config.trunks.length == 0
                          || port.vlan_config.vlan_mode === "trunk" && port.vlan_config.trunks.length >= 0) {
                      return true;
                  } else {
                      return false;
                  }
              } else {
                  return true;
              }
          },

          displayVlanTrunks: function (mode, trunks) {
              if (mode === 'access') {
                  return trunks;
              } else if (mode === 'trunk') {
                  if (trunks === null) {
                      return 'All';
                  } else if (typeof trunks === 'number') {
                      return trunks;
                  } else {
                      return trunks.join(", ");
                  }
              }
          },
          emptyList : function (resource) {
              if (resource) {
                  return resource.length === 0;
              } else {
                  return true;
              }
          },
          emptyObj : function (obj) {
              for (var prop in obj) {
                  if (obj.hasOwnProperty(prop)) {
                      return false;
                  }
              }
              return true;
          },
          defaultQueue: function (qos) {
              return qos.uuid === null;
          },
          showSpinner: function () {
              $('#loadingSpinner').modal('show');
          },
          hideSpinner: function () {
              $('#loadingSpinner').modal('hide');
          }
      }

      $scope.controlMethod = {
          isActive : function (viewLocation) {
              var v = $location.path();
              return viewLocation === v;
          }
      };
  }])
  .controller('ConfigCtrl', ['$scope', '$http', function ($scope, $http) {
      $scope.clear = function(br) {
          angular.copy($scope.br, br);
      };

      $scope.edit = function (ctrl) {
          return angular.copy(ctrl);
      };

      $scope.data = {
          firstTab : 0,
          phySwitchPage : {name:'phy', url:'/partials/phy.html'},
          nextBond : 1,
          nextTunnel : 1
      };

      $scope.toPhyTab = function () {
          $scope.panel = $scope.data.phySwitchPage;
          if ($scope.br) {
              $scope.br.name = null;
          }
      };

      (function init() {
          // The logic bridge being displayed and configured
          $scope.br = null;
          $scope.toPhyTab();
      })();

      $scope.controlMethod = {
          // Setter: everytime br is changed, the event is broadcasted.
          setBr : function (value) {
              $scope.br = value;
              $scope.$broadcast('brSynced', {br:value});
          },

          updateBr : function (value) {
              // TODO: partially update br (STP and other configs?)
              $scope.br.datapath_id = value.datapath_id;
              $scope.br.fail_mode = value.fail_mode;
              $scope.br.protocols = value.protocols;
              $scope.$broadcast('brSynced', {br:$scope.br});
          },

          displayBridge : function (brName) {
              if (!$scope.br || $scope.br.name !== brName) {
                  var promise = $http.get('/bridges/'+brName);
                  $scope.commonMethods.showSpinner();
                  promise.then(function (response) {
                      $scope.controlMethod.setBr(response.data.bridge);
                      // For ng-include: to include the basic info page
                      $scope.controlMethod.switchToTab($scope.data.firstTab);
                      // For left panel, to activate the Basic Info tab
                      $scope.$broadcast('displayBridge', $scope.data.firstTab);
                      $scope.commonMethods.hideSpinner();
                  });
              } else {
                  // For ng-include: to include the basic info page
                  $scope.controlMethod.switchToTab($scope.data.firstTab);
                  // For left panel, to activate the Basic Info tab
                  $scope.$broadcast('displayBridge', $scope.data.firstTab);
              }
          },

          // Left pane control
          left_pane : (function (starting) {
                  var config = {
                      front : starting
                  };

                  var templates = [
                      { name: 'Basic Info', url: '/partials/basic-info.html'},
                      { name: 'Controllers', url: '/partials/controller.html'},
                      { name: 'Ports', url:'/partials/ports.html'},
                      { name: 'Tunnels', url:'/partials/tunnel.html'},
                      { name: 'Link Aggregation', url:'/partials/lag.html'},
                      { name: 'Group Table', url:'/partials/group.html'},
                      { name: 'Meter Table', url:'/partials/meter.html'},
                      { name: 'Flow Table', url:'/partials/flow.html'},
                      { name: 'Mirrors', url:'/partials/mirror.html'},
                      { name: 'NetFlow', url:'/partials/netflow.html'},
                      { name: 'sFlow', url:'/partials/sflow.html'}
                  ];

                  return {
                      tab : function(index) {
                          index = (index === undefined) ? config.front : index;
                          return templates[index];
                      },
                      tabs: templates
                  }
              })(0),

          switchToTab : function (index) {
                  $scope.panel = $scope.controlMethod.left_pane.tab(index);
              },

          togglePrimary : function (id) {
              var parentDiv = $('#'+id).closest('div.panel');
              if (id !== 'hw') {
                  parentDiv.toggleClass('panel-primary');
                  parentDiv.siblings('div.panel').removeClass('panel-primary');

                  /* toggle glyphicon of my own */
                  if (parentDiv.find('span.glyphicon').hasClass('glyphicon-plus')) {
                      parentDiv.find('span.glyphicon').removeClass('glyphicon-plus').addClass('glyphicon-minus');
                      //parentDiv.find('span.glyphicon').addClass('glyphicon-minus');
                  } else if (parentDiv.find('span.glyphicon').hasClass('glyphicon-minus')) {
                      parentDiv.find('span.glyphicon').removeClass('glyphicon-minus').addClass('glyphicon-plus');
                      //parentDiv.find('span.glyphicon').addClass('glyphicon-plus');
                  }

                  /* toggle glyphicon of siblings */
                  parentDiv.siblings('div.panel:has(span)').find('span.glyphicon').removeClass('glyphicon-minus').addClass('glyphicon-plus');
              } else {
                  parentDiv.addClass('panel-primary');
                  parentDiv.siblings('div.panel').removeClass('panel-primary');

                  /* toggle glyphicon of siblings */
                  parentDiv.siblings('div.panel').find('span.glyphicon').removeClass('glyphicon-minus').addClass('glyphicon-plus');
              }
          }
      };
  }])
  .controller('MonitorCtrl', ['$scope', '$http', '$location', '$anchorScroll',
          '$timeout', function ($scope, $http, $location, $anchorScroll, $timeout) {
      $scope.$on('$locationChangeStart', function () {
          $timeout.cancel(timeoutID);
      });

      $scope.data = {
          partials: [{name: 'phySwitch', url: '/partials/phy-monitor.html'},
                  {name: 'logicBridge', url: '/partials/bridge-monitor.html'}],
          cycle: [5, 15, 30, 60, 120],
          ajaxCounter: 4
      };

      $scope.controlMethod = {
          portNum: function (offset) {
                      return function (resource) {
                                 return parseInt(resource.name.substr(offset));
                             }
          },
          loadPartial : function (index) {
                  $scope.panel = $scope.data.partials[index];
              },
          monitorBridge : function (brName) {
              if ($scope.brName !== brName) {
                  $scope.brName = brName;
                  helper.disableAutoRefresh();
                  if ($scope.brName !== 'phy switch') {
                      $scope.refresh($scope.brName);
                  } else {
                      $scope.refreshSwitchInfo();
                  }
              }
          },
          // Called on HTML partial loaded
          basicTab : function () {
              $('#topnav a:first').tab('show');
          },
          isGroupTypeFastFailover : function (group) {
              if (group.type === "fast_failover") {
                  return true;
              } else {
                  return false;
              }
          },
          actualLinkSpeed : function (state, speed) {
              if (state == "down") {
                  return "0";
              } else if (state == "up") {
                  switch (speed)
                  {
                      case 100000000000:
                          return "10 Gbps";
                          break;
                      case 1000000000:
                          return "1 Gbps";
                          break;
                      case 100000000:
                          return "100 Mbps";
                          break;
                      case 10000000:
                          return "10 Mbps";
                          break;
                      default :
                          return speed.toString() + " bps";
                  }
              }
              return "";
          },
          validOfPort : function (port) {
              return (port.ofport > 0) ? true : false;
          }
      };


      (function init() {
          $scope.controlMethod.loadPartial(0);
      })();

      $scope.brName = null;
      $scope.br = null;
      $scope.tables = [];
      $scope.groups = [];
      $scope.meters = [];
      $scope.autoRefresh = false;
      var timeoutID;

      $scope.$watch('autoRefresh', function (newValue, oldValue) {
          if (newValue !== oldValue) {
              if (newValue) {
                  // Enable auto refresh
                  var tmp = $scope.refreshCycle;
                  periodicFetch();
              } else {
                  // Disable auto refresh
                  $timeout.cancel(timeoutID);
              }
          }
      });

      // Update bridge every 2 seconds.
      var periodicFetch = function () {
          timeoutID = $timeout(function () {
              $scope.backgroundRefresh($scope.br.name);
              periodicFetch();
          }, $scope.refreshCycle * 1000);
      };

      /* Fetching bridge. */
      var helper = {
          disableAutoRefresh: function () {
              $scope.autoRefresh = false;
          },

          get_bridge : function (brname) {
              var promise = $http.get('/bridges/'+brname);
              promise.then(function (response) {
                  $scope.br = response.data.bridge;
                  $scope.data.ajaxCounter -= 1;
              });
          },

          /* Fetching flows. */
          get_flows : function (brname) {
              var promise = $http.get('/bridges/'+brname+'/tables');
              promise.then(function(response) {
                  $scope.tables = response.data.tables;
                  $scope.data.ajaxCounter -= 1;
              });
          },

          /* Fetching groups. */
          get_groups : function (brname) {
              var promise = $http.get('/bridges/'+brname+'/groups');
              promise.then(function(response) {
                  $scope.groups = response.data.groups;
                  $scope.data.ajaxCounter -= 1;
              });
          },

          /* Fetching meters. */
          //$scope.get_meters = function (brname) {
          get_meters : function (brname) {
              var promise = $http.get('/bridges/'+brname+'/meters');
              promise.then(function(response) {
                  $scope.meters = response.data.meters;
                  $scope.data.ajaxCounter -= 1;
              });
          }
      };

      $scope.backgroundRefresh = function (brname) {
          helper.get_bridge(brname);
          helper.get_flows(brname);
          helper.get_groups(brname);
          helper.get_meters(brname);
      };

      $scope.refresh = function (brname) {
          $scope.commonMethods.showSpinner();

          $scope.$watch('data.ajaxCounter', function (newValue, oldValue) {
              if (newValue != oldValue) {
                  if (newValue === 0) {
                      $scope.commonMethods.hideSpinner();
                      $scope.data.ajaxCounter = 4;
                  }
              }
          });

          helper.get_bridge(brname);
          helper.get_flows(brname);
          helper.get_groups(brname);
          helper.get_meters(brname);
      };

      $scope.scrollTo = function(id) {
          var old = $location.hash();
          $location.hash(id);
          $anchorScroll();
          //reset to old to keep any additional routing logic from kicking in
          $location.hash(old);
      };

      $scope.ctrlTarget = function (ctl) {
          if (ctl.protocol === "tcp") {
              return ctl.protocol + ":" + ctl.ip + ":" + ctl.port.toString();
          } else {
              return ctl.protocol + ":" + ctl.port.toString() + ":" + ctl.ip;
          }
      };
  }])
  .controller('HelpCtrl', [function () {
  }])
  .controller('BasicInfoCtrl', ['$scope', '$http', function ($scope, $http) {
      $scope.data = {
          localBr: {},
          ofVersion: [
              {desc:'OpenFlow10', isSelected:false},
              {desc:'OpenFlow12', isSelected:false},
              {desc:'OpenFlow13', isSelected:false}
          ],
          failMode: ['secure', 'standalone']
      };

      var helper = {
          // "OpenFlow10" OR ["OpenFlow10", "OpenFlow13"] => ofVersion
          dataFromServer: function (protocols) {
              var ofs = ['OpenFlow10', 'OpenFlow12', 'OpenFlow13'];
              var myVersion = [
                  {desc:'OpenFlow10', isSelected:false},
                  {desc:'OpenFlow12', isSelected:false},
                  {desc:'OpenFlow13', isSelected:false}
              ];
              var protocol, i, j;

              if (protocols !== null) {
                  if (typeof(protocols) === "object") {
                      for (i = 0; i < protocols.length; i++) {
                          j = ofs.indexOf(protocols[i]);
                          myVersion[j].isSelected = true;
                      }
                  } else {
                      i = ofs.indexOf(protocols);
                      myVersion[i].isSelected = true;
                  }
              }

              return myVersion;
          },
          // ofVersion => $scope.br.protocols
          data2Server: function (capability) {
              var result = [];

              for (var i in capability) {
                  if (capability[i].isSelected) {
                      result.push(capability[i].desc);
                  }
              }

              return result.join();
          }
      };

      (function init() {
          $scope.$on('brSynced', function (event, args) {
              angular.copy(args.br, $scope.data.localBr);
              $scope.data.ofVersion = helper.dataFromServer($scope.data.localBr.protocols);
          });

          // Special case: navigating from other tabs
          if (!angular.equals($scope.br, $scope.data.localBr)) {
              if ($scope.br) {
                  angular.copy($scope.br, $scope.data.localBr);
                  $scope.data.ofVersion = helper.dataFromServer($scope.data.localBr.protocols);
              }
          }
      })();

      $scope.crud = {
          create: null,
          read: null,
          update: function (record) {
              // update OpenFlow versions
              record.protocols = helper.data2Server($scope.data.ofVersion);

              var success = {type: "success", msg: "Changes are being applied. Please wait before they take effect."};
              $scope.alerts.push(success);

              var promise = $http.post('/bridges/'+$scope.br.name+'/update', record);
              promise.then(function(response) {
                  if (response.data.ret === 0) {
                      $scope.controlMethod.updateBr(response.data.bridge);
                  } else {
                      var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                      $scope.alerts.push(failure);
                  }
              });
          },
          del: null
      };
  }])
  .controller('FlowTableCtrl', ['$scope', '$http', function ($scope, $http) {
    $scope.data = {
        tables: [],
        matchFields: [
        {field:'in_port', layer:'Others'},
        {field:'cookie', layer:'Others'},
        {field:'priority', layer:'Others'},
        {field:'idle_timeout', layer:'Others'},
        {field:'hard_timeout', layer:'Others'},
        {field:'send_flow_rem', layer:'Others'},
        {field:'dl_vlan', layer:'Layer 2'},
        {field:'dl_vlan_pcp', layer:'Layer 2'},
        {field:'dl_src', layer:'Layer 2'},
        {field:'dl_dst', layer:'Layer 2'},
        {field:'dl_type', layer:'Layer 2'},
        {field:'mpls_label', layer:'Layer 2.5'},
        {field:'mpls_tc', layer:'Layer 2.5'},
        {field:'arp_spa', layer:'Layer 2.5'},
        {field:'arp_tpa', layer:'Layer 2.5'},
        {field:'nw_src', layer:'Layer 3'},
        {field:'nw_dst', layer:'Layer 3'},
        {field:'nw_proto', layer:'Layer 3'},
        {field:'nw_tos', layer:'Layer 3'},
        {field:'nw_ecn', layer:'Layer 3'},
        // {field:'nw_ttl', layer:'Layer 3'},
        {field:'tp_src', layer:'Layer 4'},
        {field:'tp_dst', layer:'Layer 4'},
        {field:'icmp_type', layer:'Layer 4'},
        {field:'icmp_code', layer:'Layer 4'},
        // {field:'arp_sha', layer:'ARP'},
        // {field:'arp_tha', layer:'ARP'},
        {field:'ipv6_src', layer:'IPv6'},
        {field:'ipv6_dst', layer:'IPv6'},
        // {field:'ipv6_label', layer:'IPv6'},
        // {field:'nd_target', layer:'IPv6'},
        // {field:'nd_sll', layer:'IPv6'},
        // {field:'nd_tll', layer:'IPv6'},
        // {field:'tun_id', layer:'tunnel'},
        // {field:'tun_src', layer:'tunnel'},
        // {field:'tun_dst', layer:'tunnel'},
    ],
        ofActions: [
        {type:'Supported by hareware', action:'drop'},
        {type:'Supported by hareware', action:'all'},
        {type:'Supported by hareware', action:'flood'},
        {type:'Supported by hareware', action:'normal'},
        {type:'Supported by hareware', action:'output'},
        {type:'Supported by hareware', action:'enqueue'},
        {type:'Supported by hareware', action:'controller'},
        {type:'Supported by hareware', action:'mod_vlan_vid'},
        {type:'Supported by hareware', action:'mod_vlan_pcp'},
        {type:'Supported by hareware', action:'mod_dl_src'},
        {type:'Supported by hareware', action:'mod_dl_dst'},
        {type:'Supported by hareware', action:'strip_vlan'},
        {type:'Supported by hareware', action:'push_vlan'},
        {type:'Supported by hareware', action:'pop_vlan'},
        {type:'Supported by hareware', action:'push_pbb'},
        {type:'Supported by hareware', action:'pop_pbb'},
        {type:'Supported by hareware', action:'push_mpls'},
        {type:'Supported by hareware', action:'pop_mpls'},
        {type:'Supported by hareware', action:'set_mpls_ttl'},
        {type:'Supported by hareware', action:'set_queue'},
        {type:'Supported by hareware', action:'set_field'},
        {type:'Supported by hareware', action:'resubmit'},
        {type:'Supported by hareware', action:'group'},
        {type:'Supported by hareware', action:'meter'},
        {type:'Supported by hareware', action:'goto_table'},
        {type:'Non-hardware Support', action:'mod_nw_src'},
        {type:'Non-hardware Support', action:'mod_nw_dst'},
        {type:'Non-hardware Support', action:'mod_nw_tos'},
        {type:'Non-hardware Support', action:'mod_tp_src'},
        {type:'Non-hardware Support', action:'mod_tp_dst'},
    ],
        newFlow: {
            tableID: null,
            field: null,
            value: null,
            matches: null,
            action: null,
            parameter: null,
            actions: null,
            fullFlow: {}
        },
        flowDescValidity: "Match field description is valid.",
        actionsDescValidity: "List of actions is valid."
    };

   $scope.controlMethod = {
        noneEmpty: function (ele) {
                       if (ele.flows.length == 0)
                           return false;
                       else
                           return true;
                   },
        add_field: function (match, value) {
                       if (value === null || value === "") {
                           $scope.data.newFlow.matches += match.field+",";
                       } else {
                           $scope.data.newFlow.matches += match.field+"="+value+",";
                       }
                   },
        add_action: function (act, par) {
                        if (par === null || par === "") {
                            $scope.data.newFlow.actions += act.action+",";
                        } else if (act.action === "controller") {
                            $scope.data.newFlow.actions += act.action+par+",";
                        } else {
                            $scope.data.newFlow.actions += act.action+":"+par+",";
                        }
                    },
        init: function () {
                  $scope.data.newFlow.tableID = 0;
                  $scope.data.newFlow.field = null;
                  $scope.data.newFlow.value = null;
                  $scope.data.newFlow.action = null;
                  $scope.data.newFlow.parameter = null;
                  $scope.data.newFlow.matches = "";
                  $scope.data.newFlow.actions = "";
                  $scope.data.newFlow.fullFlow.flow = "table=" + $scope.data.newFlow.tableID + "," + $scope.data.newFlow.matches + "actions=" + $scope.data.newFlow.actions;
              },
        setFlowDescValidity: function (msg) {
            $scope.data.flowDescValidity = msg;
        },
        setActionsDescValidity: function (msg) {
            $scope.data.actionsDescValidity = msg;
        }
    };
    var helper = {
        table_index: function (id) {
                         for (var i = 0; i < $scope.data.tables.length; i++) {
                             if (id == $scope.data.tables[i].id)
                                 return i;
                         }
                         return -1;
                     }
    };

    $scope.crud = {
        batchCreate: function (record) {
                         var promise = $http.post('/bridges/'+$scope.br.name+'/flows', record);
                         promise.then(function(response){
                             if (response.data.ret === 0) {
                                 var success = {type: "success", msg: "New flows are successfully added."};
                                 $scope.data.tables = response.data.tables;
                             } else {
                                var failure = {type: "danger", msg: "Oops! Check things up and try again: " + response.data.msg};
                                // var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                                $scope.alerts.push(failure);
                             }
                         });
                     },
        create: function (record) {
                    var promise = $http.post('/bridges/'+$scope.br.name+'/tables/'+$scope.data.newFlow.tableID+'/flows/add', record);
                    promise.then(function(response){
                        if (response.data.ret === 0) {
                            var success = {type: "success", msg: "A new flow is successfully added."};
                            var table = response.data.table;
                            var index = helper.table_index(table.id);
                            if (index >= 0)
                                $scope.data.tables[index].flows = table.flows;
                            else
                                $scope.data.tables.push(table);
                            $scope.alerts.push(success);
                        } else {
                            var failure = {type: "danger", msg: "Oops! Check things up and try again: " + response.data.msg};
                            $scope.alerts.push(failure);
                        }
                    });
                },
        read: function () {
                  var promise = $http.get('/bridges/'+$scope.br.name+'/tables');
                  $scope.commonMethods.showSpinner();
                  promise.then(function(response) {
                      $scope.data.tables = response.data.tables;
                      $scope.commonMethods.hideSpinner();
                  });
              },
        update: function (record) {
                    var promise = $http.post('/bridges/'+$scope.br.name+'/tables/'+record.table+'/flows/update', record);
                    promise.then(function(response){
                        if (response.data.ret === 0) {
                            var success = {type: "success", msg: "The flow has been updated."};
                            var table = response.data.table;
                            var index = helper.table_index(table.id);
                            if (index >= 0)
                                $scope.data.tables[index].flows = table.flows;
                            $scope.alerts.push(success);
                        } else {
                            var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                            $scope.alerts.push(failure);
                        }
                    });
                },
        del: function (record) {
                 var promise = $http.post('/bridges/'+$scope.br.name+'/tables/'+record.table+'/flows/del', record);
                 promise.then(function(response){
                     if (response.data.ret === 0) {
                         var success = {type: "success", msg: "Flow is removed."};
                         var table = response.data.table;
                         var index = helper.table_index(table.id);
                         if (index >= 0) {
                             $scope.data.tables[index].flows = table.flows;
                             if (table.flows.length == 0)
                                 $scope.data.tables.splice(index, 1);
                         }
                         $scope.alerts.push(success);
                     } else {
                         var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                         $scope.alerts.push(failure);
                     }
                 });
             },
        clear: function (tableId) {
                 var promise = $http.post('/bridges/'+$scope.br.name+'/tables/'+tableId+'/flows/clear', {});
                 promise.then(function(response){
                     if (response.data.ret === 0) {
                         var success = {type: "success", msg: "Flow Table is removed."};
                         var table = response.data.table;
                         var index = helper.table_index(table.id);
                         if (index >= 0) {
                             $scope.data.tables[index].flows = table.flows;
                             if (table.flows.length == 0)
                                 $scope.data.tables.splice(index, 1);
                         }
                         $scope.alerts.push(success);
                     } else {
                         var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                         $scope.alerts.push(failure);
                     }
                 });

        }
    };

    (function init () {
        function sync (newFlow) {
                newFlow.fullFlow.flow = "table=" + newFlow.tableID + "," + newFlow.matches + "actions=" + newFlow.actions;
        }

        $scope.$watch("data.newFlow.matches", function(newValue, oldValue) {
            if (newValue !== oldValue) {
                sync($scope.data.newFlow);
            }
        });

        $scope.$watch("data.newFlow.actions", function(newValue, oldValue) {
            if (newValue !== oldValue) {
                sync($scope.data.newFlow);
            }
        });

        $scope.$watch("data.newFlow.tableID", function(newValue, oldValue) {
            if (newValue !== oldValue) {
                sync($scope.data.newFlow);
            }
        });

        $scope.crud.read();
    })();
  }])
  .controller('TunnelCtrl', ['$scope', '$http', function ($scope, $http) {
    $scope.data = {
        tunnels: $scope.br.tunnels,
        availableTypes: ['GRE', 'NVGRE', 'VXLAN'],
        enabledType: 'GRE',
        phyPorts: [],
        tunnelsOptions: []
    };

    $scope.initializer = {
        newTunnel: function () {
                       var tunnel = {};
                       return {
                           init: function () {
                                     tunnel.type = null;
                                     tunnel.name = null;
                                     tunnel.options = {
                                         "remote_ip":null,
                                         "local_ip":null,
                                         "src_mac":null,
                                         "dst_mac":null,
                                         "egress_port":null,
                                         "vlan":null
                                     };
                                     return tunnel;
                                 }
                       }
                   }
    };

    (function init() {
        for (var i = 0; i < $scope.br.ports.length; i++) {
            if ($scope.br.ports[i].ofport > 0)
                $scope.data.phyPorts.push($scope.br.ports[i].name);
        }

        for (var i = 1; i < 33; i++) {
            $scope.data.tunnelsOptions.push('gre'+i.toString());
        };

        $scope.$watch('data.tunnels', function (newValue, oldValue) {
            if (newValue !== undefined && newValue !== null) {
                var names = [];
                if (newValue.length === 0) {
                    var oldTunnelID = $scope.$parent.data.nextTunnel;
                    $scope.$parent.data.nextTunnel = oldTunnelID > 1 ? oldTunnelID : 1
                } else {
                    for (var i = 0; i < newValue.length; i++) {
                        names.push(newValue[i].name);
                    }
                    names.sort(function(a,b){
                        return (Number(a.substr(3)) - Number(b.substr(3)));
                    });
                    var last = names.pop();
                    var oldTunnelID = $scope.$parent.data.nextTunnel;
                    var newTunnelID = parseInt(last.substr(3)) + 1;
                    $scope.$parent.data.nextTunnel = newTunnelID > oldTunnelID ? newTunnelID : oldTunnelID;
                }
            }
        });
    })();

    $scope.controlMethod = {
        tunnelNum: function (tunnel) {
                     return parseInt(tunnel.name.substr(3));
                 },
        portNum: function (id) {
                     return parseInt(id.split('/')[2]);
                 },
        typeFilter: function (ele) {
                        var tunnelTypes = {
                            GRE: "webovs_gre",
                            NVGRE: "webovs_nvgre",
                            VXLAN: "webovs_vxlan"
                        };

                        if (ele.type === tunnelTypes[$scope.data.enabledType]) {
                            return true;
                        } else {
                            if ($scope.data.enabledType === 'GRE' && ele.type === 'pronto_gre') {
                                return true;
                            } else {
                                return false;
                            }
                        }
                    }
    };

    $scope.crud = {
        create: function (record, type) {
                    var tunnelTypes = {
                        GRE: "webovs_gre",
                        NVGRE: "webovs_nvgre",
                        VXLAN: "webovs_vxlan"
                    };
                    record.type = tunnelTypes[type];
                    // record.name = type.toLowerCase() + $scope.$parent.data.nextTunnel.toString();
                    var promise = $http.post('/bridges/'+$scope.br.name+'/tunnels/'+record.name+'/add', record);
                    var success = {type: "success", msg: "A new tunnel is being created. Please wait until it appears below."};
                    $scope.alerts.push(success);
                    promise.then(function(response) {
                        if (response.data.ret === 0) {
                            $scope.data.tunnels = response.data.tunnels;
                            $scope.br.tunnels = $scope.data.tunnels;
                        } else {
                            var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                            $scope.alerts.push(failure);
                        }
                    });
                },
        read: null,
        update: function (record) {
                    var promise = $http.post('/bridges/'+$scope.br.name+'/tunnels/'+record.name+'/update', record);
                    var success = {type: "success", msg: "Changes are being applied. Please wait before they take effect."};
                    $scope.alerts.push(success);
                    promise.then(function(response) {
                        if (response.data.ret === 0) {
                            $scope.data.tunnels = response.data.tunnels;
                            $scope.br.tunnels = $scope.data.tunnels;
                        } else {
                            var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                            $scope.alerts.push(failure);
                        }
                    });
                },
        del: function (record) {
                 var promise = $http.post('/bridges/'+$scope.br.name+'/tunnels/'+record.name+'/del', record);
                 var success = {type: "success", msg: "A tunnel is being destroyed. Please wait until it disappears from the list below."};
                 $scope.alerts.push(success);
                 promise.then(function(response) {
                     if (response.data.ret === 0) {
                         $scope.data.tunnels = response.data.tunnels;
                         $scope.br.tunnels = $scope.data.tunnels;
                     } else {
                         var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                         $scope.alerts.push(failure);
                     }
                 });
             }
    };




/*
 *==================================================
 */
    /*
    $scope.tunnels = $scope.br.tunnels;
    $scope.availableTypes = ['GRE', 'NVGRE', 'VXLAN'];
    $scope.enabledType = 'GRE';

    $scope.newTunnel = function () {
        var tunnel = {};
        return {
            init: function () {
                      tunnel.type = null;
                      tunnel.name = null;
                      tunnel.options = {
                          "remote_ip":null,
                          "local_ip":null,
                          "src_mac":null,
                          "dst_mac":null,
                          "egress_port":null,
                          "vlan":null
                      };
                      return tunnel;
                  }
        }
    };

    $scope.phyPorts = [];
    for (var i = 0; i < $scope.br.ports.length; i++) {
        $scope.phyPorts.push($scope.br.ports[i].name);
    }

    $scope.name = '+name';
    $scope.portNum = function (id) {
        return id;
    };
    $scope.typeFilter = function (ele) {
        var tunnelTypes = {
            GRE: "webovs_gre",
            NVGRE: "webovs_nvgre",
            VXLAN: "webovs_vxlan"
        };

        if (ele.type === tunnelTypes[$scope.enabledType]) {
            return true;
        } else {
            return false;
        }
    };

    $scope.nextTunnel = 1;

    $scope.$watch('tunnels', function (newValue, oldValue) {
        if (newValue !== undefined && newValue !== null) {
            var names = [];
            if (newValue.length === 0) {
                $scope.nextTunnel = 1;
            } else {
                for (var i = 0; i < newValue.length; i++) {
                    names.push(newValue[i].name);
                }
                names.sort();
                var last = names.pop();
                $scope.nextTunnel = parseInt(last.substr(3)) + 1;
            }
        }
    });

    $scope.update_tunnel = function(record) {
        var promise = $http.post('/bridges/'+$scope.br.name+'/tunnels/'+record.name+'/update', record);
        promise.then(function(response) {
            if (response.data.ret === 0) {
                var success = {type: "success", msg: "Successfully updated."};
                $scope.tunnels = response.data.tunnels;
                $scope.br.tunnels = $scope.tunnels;
                $scope.alerts.push(success);
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }
        });
    };

    $scope.del_tunnel = function(record) {
        var promise = $http.post('/bridges/'+$scope.br.name+'/tunnels/'+record.name+'/del', record);
        promise.then(function(response) {
            $scope.tunnels = response.data.tunnels;
            $scope.br.tunnels = $scope.tunnels;
        });
    };

    $scope.add_tunnel = function(record, type) {
        var tunnelTypes = {
            GRE: "webovs_gre",
            NVGRE: "webovs_nvgre",
            VXLAN: "webovs_vxlan"
        };
        record.type = tunnelTypes[type];
        record.name = type.toLowerCase() + $scope.nextTunnel.toString();
        var promise = $http.post('/bridges/'+$scope.br.name+'/tunnels/'+record.name+'/add', record);
        promise.then(function(response) {
            if (response.data.ret === 0) {
                var success = {type: "success", msg: "Successfully added."};
                $scope.tunnels = response.data.tunnels;
                $scope.br.tunnels = $scope.tunnels;
                $scope.alerts.push(success);
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }
        });
    };
    */
  }])
  .controller('ControllerCtrl', ['$scope', '$http', function ($scope, $http) {
    //$scope.controllers = $scope.br.controller;
    $scope.data = {
        controllers: $scope.br.controller,
        methods: ['tcp'],
        mode: ['in-band', 'out-of-band']
    };

    $scope.initializer = {
        newCtrl: function () {
                     var ctrl = {};
                     return {
                         init: function () {
                                   ctrl.protocol = "tcp";
                                   ctrl.ip = null;
                                   ctrl.port = 6633;
                                   ctrl.connection_mode = "out-of-band";
                                   return ctrl
                               }
                     }
                 }
    };

    $scope.crud = {
        create: function (record) {
                    var promise = $http.post('/bridges/'+$scope.br.name+'/controllers/add', record);
                    var success = {type: "success", msg: "A new controller is being configured. Please wait until it appears below."};
                    $scope.alerts.push(success);
                    promise.then(function(response) {
                        if (response.data.ret === 0) {
                            $scope.data.controllers = response.data.controllers;
                            $scope.br.controller = $scope.data.controllers;
                        } else {
                            var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                            $scope.alerts.push(failure);
                        }
                    });
                },
        read: null,
        update: function (record) {
                    var promise = $http.post('/bridges/'+$scope.br.name+'/controllers/update', record);
                    var success = {type: "success", msg: "Changes are being applied. Please wait before they take effect."};
                    $scope.alerts.push(success);
                    promise.then(function(response) {
                        if (response.data.ret === 0) {
                            $scope.data.controllers = response.data.controllers;
                            $scope.br.controller = $scope.data.controllers;
                        } else {
                            var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                            $scope.alerts.push(failure);
                        }
                    });
                },
        del: function (record) {
                 var promise = $http.post('/bridges/'+$scope.br.name+'/controllers/del', record);
                 var success = {type: "success", msg: "A controller is being removed. Please wait until it disappears from the list below."};
                 $scope.alerts.push(success);
                 promise.then(function(response) {
                     if (response.data.ret === 0) {
                         $scope.data.controllers = response.data.controllers;
                         $scope.br.controller = $scope.data.controllers;
                     } else {
                         var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                         $scope.alerts.push(failure);
                     }
                 });
             }
    };
  }])
  .controller('GroupCtrl', ['$scope', '$http', function ($scope, $http) {
      (function init () {
          var promise = $http.get('/bridges/'+$scope.br.name+'/groups');
          $scope.commonMethods.showSpinner();
          promise.then(function(response) {
              $scope.groups = response.data.groups;
              $scope.commonMethods.hideSpinner();
          });
      })();

    $scope.data = {
        types : ['all', 'select', 'indirect', 'fast_failover']
    };

    /* Member selection */
    $scope.controlMethod = {
        phyPortIDs : function (ports) {
            var portIDs = [];
            for (var i = 0; i < ports.length; i++) {
                portIDs.push(parseInt(ports[i].name.split('/')[2]));
            }
            portIDs.sort(function(a,b){return a-b});
            return portIDs;
        },

        groupIDs : function (groups) {
            var ids = [];
            if (groups) {
                for (var i = 0; i < groups.length; i++) {
                    ids.push($scope.groups[i].id);
                }
            }
            ids.sort(function(a,b){return a-b});
            return ids;
        },

        //portNum : function (id) {
            //return id;
        //},

        isTypeFastFailover : function (type) {
            return (type === "fast_failover");
        },

        newGroup :  function () {
            var group = {};
            return {
                init: function () {
                          group.id = null;
                          group.type = "all";
                          group.counter = {"packets_count":null, "bytes_count":null};
                          group.buckets = [];
                          group.ref_count = null;
                          group.duration = null;
                          return group;
                      }
            }
        }
    };

    $scope.crud = {
        create: function(record) {
            var promise = $http.post('/bridges/'+$scope.br.name+'/groups/'+record.id+'/add', record);
            promise.then(function(response) {
                if (response.data.ret === 0) {
                    var success = {type: "success", msg: "A new group table entry is successfully added."};
                    $scope.groups = response.data.groups;
                    $scope.br.groups = $scope.groups;
                    $scope.alerts.push(success);
                } else {
                    var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                    $scope.alerts.push(failure);
                }
            });
        },

        read: null,

        update: function (record) {
            var promise = $http.post('/bridges/'+$scope.br.name+'/groups/'+record.id+'/update', record);
            promise.then(function(response) {
                  if (response.data.ret === 0) {
                      var success = {type: "success", msg: "Successfully updated."};
                      $scope.groups = response.data.groups;
                      $scope.br.groups = $scope.groups;
                      $scope.alerts.push(success);
                  } else {
                      var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                      $scope.alerts.push(failure);
                  }
            });
        },

        del: function (record) {
            var promise = $http.post('/bridges/'+$scope.br.name+'/groups/'+record.id+'/del', record);
            promise.then(function(response) {
                if (response.data.ret === 0) {
                    var success = {type: "success", msg: "Group is removed."};
                    $scope.groups = response.data.groups;
                    $scope.br.groups = $scope.groups;
                    $scope.alerts.push(success);
                } else {
                    var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                    $scope.alerts.push(failure);
                }
            });
        },

        clear: function (unused) {
            var promise = $http.delete('/bridges/'+$scope.br.name+'/groups');
            promise.then(function(response) {
                if (response.data.ret === 0) {
                    var success = {type: "success", msg: "All groups have been removed."};
                    $scope.groups = response.data.groups;
                    $scope.br.groups = $scope.groups;
                    $scope.alerts.push(success);
                } else {
                    var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                    $scope.alerts.push(failure);
                }
            });
        }
    };
  }])
  .controller('BondCtrl', ['$scope', '$http', function ($scope, $http) {
    $scope.data = {
        bonds: $scope.br.lags,
        options: {
            lags: [],
            speed: ["auto", "10G", "1G", "100M", "10M"],
            vlanModes: ["access", "trunk"],
            lacpModes: ["active", "passive"],
            lagTypes: ["static", "lacp"],
            lacpTime: ["fast", "slow"]
        },
        phyPorts: []
    };

    (function init() {
        for (var i = 0; i < $scope.br.ports.length; i++) {
            if ($scope.br.ports[i].ofport > 0)
                $scope.data.phyPorts.push($scope.br.ports[i].name);
        }
        for (var i = 1; i < 25; i++) {
            $scope.data.options.lags.push("ae" + i.toString());
        };

        $scope.$watch('data.bonds', function (newValue, oldValue) {
            if (newValue !== undefined && newValue !== null) {
                var names = [];
                if (newValue.length === 0) {
                    var oldBondID = $scope.$parent.data.nextBond;
                    $scope.$parent.data.nextBond = oldBondID > 1 ? oldBondID : 1
                } else {
                    for (var i = 0; i < newValue.length; i++) {
                        names.push(newValue[i].name);
                    }
                    names.sort(function(a,b){
                        return (Number(a.substr(2)) - Number(b.substr(2)));
                    });
                    var last = names.pop();
                    var oldBondID = $scope.$parent.data.nextBond;
                    var newBondID = parseInt(last.substr(2)) + 1;
                    $scope.$parent.data.nextBond = newBondID > oldBondID ? newBondID : oldBondID;
                }
            }
        });
    })();

    $scope.initializer = {
        newBond: function () {
                     var bond = {};
                     return {
                         init: function () {
                                   bond.name = null;
                                   bond.options = {};
                                   bond.vlan_config = {};
                                   bond.options["lacp-mode"] = "active";
                                   bond.options["lacp-system-id"] = null;
                                   bond.options["lacp-system-priority"] = "32768";
                                   bond.options["lacp-time"] = "slow";
                                   bond.options.lag_type = "static";
                                   bond.options.members = [];
                                   bond.type = "webovs_lag";
                                   bond.vlan_config.tag = 1;
                                   bond.vlan_config.vlan_mode = "access";
                                   bond.vlan_config.trunks = [];
                                   return bond;
                               }
                     }
                 }
    };

    $scope.controlMethod = {
        lagNum: function (bond) {
                     return parseInt(bond.name.substr(2));
                 },
        edit: function (port) {
                      var myport = angular.copy(port);
                      if (myport.vlan_config.trunks === null) {
                          myport.vlan_config.trunks = [];
                      }
                      return myport;
                  },
        portNum: function (id) {
                     return parseInt(id.split('/')[2]);
                 }
    };

    $scope.crud = {
        create: function (record) {
                    var promise = $http.post('/bridges/'+$scope.br.name+'/bonds/'+record.name+'/add', record);
                    var success = {type: "success", msg: "A new bond is being created. Please wait until it appears below."};
                    $scope.alerts.push(success);
                    promise.then(function(response) {
                          if (response.data.ret === 0) {
                              $scope.data.bonds = response.data.bonds;
                              $scope.br.lags = $scope.data.bonds;
                          } else {
                              var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                              $scope.alerts.push(failure);
                          }
                    });
                },
        read: null,
        update: function (record) {
                    var promise = $http.post('/bridges/'+$scope.br.name+'/bonds/'+record.name+'/update', record);
                    var success = {type: "success", msg: "Changes are being applied. Please wait before they take effect."};
                    $scope.alerts.push(success);
                    promise.then(function(response) {
                        if (response.data.ret === 0) {
                            $scope.data.bonds = response.data.bonds;
                        } else {
                            var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                            $scope.alerts.push(failure);
                        }
                    });
                },
        del: function (record) {
                 var promise = $http.post('/bridges/'+$scope.br.name+'/bonds/'+record.name+'/del', record);
                 var success = {type: "success", msg: "A bond is being destroyed. Please wait until it disappears from the list below."};
                 $scope.alerts.push(success);
                 promise.then(function(response) {
                     if (response.data.ret === 0) {
                         $scope.data.bonds = response.data.bonds;
                     } else {
                         var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                         $scope.alerts.push(failure);
                     }
                 });
             }
    };
  }])
  .controller('sFlowCtrl', ['$scope', '$http', function ($scope, $http) {
    $scope.sflow = $scope.br.sFlow;

    $scope.newsFlow = function () {
        var sflow = {};
        return {
            init: function () {
                      sflow.polling = 20;
                      sflow.header = 128;
                      sflow.targets = [];
                      sflow.agent = "eth0";
                      sflow.sampling = 128;
                      return sflow;
                  }
        }
    };

    $scope.editOrAdd = function (mysflow) {
        //Return true if mysflow is an empty object.
        return (mysflow && (Object.keys(mysflow).length > 0)) ? true : false;
    };

    $scope.update_sflow = function(record){
        var promise = $http.post('/bridges/'+$scope.br.name+'/sflow/update', record);
        var success = {type: "success", msg: "Changes are being applied. Please wait before they take effect."};
        $scope.alerts.push(success);
        promise.then(function(response){
            if (response.data.ret === 0) {
                $scope.sflow = response.data.sflow;
                $scope.br.sFlow = $scope.sflow;
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }
        });
    };

    $scope.del_sflow = function(record){
        var promise = $http.post('/bridges/'+$scope.br.name+'/sflow/del', record);
        var success = {type: "success", msg: "sFlow configuration is being cleared. Please wait until it disappears."};
        $scope.alerts.push(success);
        promise.then(function(response){
            if (response.data.ret === 0) {
                $scope.sflow = response.data.sflow;
                $scope.br.sFlow = $scope.sflow;
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }
        });
    };

    $scope.add_sflow = function(record){
        record.targets = record.targets.slice(0, $scope.target_count);
        var promise = $http.post('/bridges/'+$scope.br.name+'/sflow/add', record);
        var success = {type: "success", msg: "sFlow is being configured. Please wait until it appears below."};
        $scope.alerts.push(success);
        promise.then(function(response){
            if (response.data.ret === 0) {
                $scope.sflow = response.data.sflow;
                $scope.br.sFlow = $scope.sflow;
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }
        });
    };
  }])
  .controller('NetFlowCtrl', ['$scope', '$http', function ($scope, $http) {
    $scope.netflow = $scope.br.NetFlow;

    $scope.newNetFlow = function () {
        var netflow = {};
        return {
            init: function () {
                      netflow.active_timeout = 30;
                      netflow.targets = [];
                      return netflow;
                  }
        }
    };

    $scope.editOrAdd = function (mysflow) {
        //Return true if mysflow is an empty object.
        return (mysflow && (Object.keys(mysflow).length > 0)) ? true : false;
    };

    $scope.update_netflow = function(record){
        var promise = $http.post('/bridges/'+$scope.br.name+'/netflow/update', record);
        var success = {type: "success", msg: "Changes are being applied. Please wait before they take effect."};
        $scope.alerts.push(success);
        promise.then(function(response){
            if (response.data.ret === 0) {
                $scope.netflow = response.data.netflow;
                $scope.br.NetFlow = $scope.netflow;
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }
        });
    };

    $scope.del_netflow = function(record){
        var promise = $http.post('/bridges/'+$scope.br.name+'/netflow/del', record);
        var success = {type: "success", msg: "NetFlow configuration is being cleared. Please wait until it disappears."};
        $scope.alerts.push(success);
        promise.then(function(response){
            if (response.data.ret === 0) {
                $scope.netflow = response.data.netflow;
                $scope.br.NetFlow = $scope.netflow;
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }
        });
    };

    $scope.add_netflow = function(record){
        record.targets = record.targets.slice(0, $scope.target_count);
        var promise = $http.post('/bridges/'+$scope.br.name+'/netflow/add', record);
        var success = {type: "success", msg: "NetFlow is being configured. Please wait until it appears below."};
        $scope.alerts.push(success);
        promise.then(function(response){
            if (response.data.ret === 0) {
                $scope.netflow = response.data.netflow;
                $scope.br.NetFlow = $scope.netflow;
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }
        });
    };
  }])
  .controller('MirrorCtrl', ['$scope', '$http', function ($scope, $http) {
    $scope.mirrors = $scope.br.Mirrors;

    $scope.newMirror = function () {
        var mirror = {};
        return {
            init: function () {
                      mirror.name = null;
                      mirror.select_src_port = [];
                      mirror.select_dst_port = [];
                      mirror.output_port = null;
                      mirror.select_vlan = [];
                      mirror.output_vlan = null;
                      return mirror;
              }
        }
    };

    /* Member selection */
    $scope.phyPorts = [];
    for (var i = 0; i < $scope.br.ports.length; i++) {
        if ($scope.br.ports[i].ofport > 0)
            $scope.phyPorts.push($scope.br.ports[i].name);
    }

    $scope.portNum = function (id) {
        return parseInt(id.split('/')[2]);
    };

    $scope.update_mirror = function (record) {
        var promise = $http.post('/bridges/'+$scope.br.name+'/mirrors/'+record.name+'/update', record);
        var success = {type: "success", msg: "Changes are being applied. Please wait before they take effect."};
        $scope.alerts.push(success);
        promise.then(function(response) {
            if (response.data.ret === 0) {
                $scope.mirrors = response.data.mirrors;
                $scope.br.Mirrors = $scope.mirrors;
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }
        });
    };

    $scope.del_mirror = function (record) {
        var promise = $http.post('/bridges/'+$scope.br.name+'/mirrors/'+record.name+'/del', record);
        var success = {type: "success", msg: "A mirror is being destroyed. Please wait until it disappears from the list below."};
        $scope.alerts.push(success);
        promise.then(function(response) {
            if (response.data.ret === 0) {
                $scope.mirrors = response.data.mirrors;
                $scope.br.Mirrors = $scope.mirrors;
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }
        });
    };

    $scope.add_mirror = function(record) {
        var promise = $http.post('/bridges/'+$scope.br.name+'/mirrors/'+record.name+'/add', record);
        var success = {type: "success", msg: "A new mirror is being created. Please wait until it appears below."};
        $scope.alerts.push(success);
        promise.then(function(response) {
            if (response.data.ret === 0) {
                $scope.mirrors = response.data.mirrors;
                $scope.br.Mirrors = $scope.mirrors;
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }
        });
    };
  }])
  .controller('PortsCtrl', ['$scope', '$http', '$timeout', function ($scope, $http, $timeout) {
    $scope.port = function (name) {
        return function (element, index) {
            return element.name === name;
        }
    };

    $scope.data = {
        ports: $scope.br.ports,
        options: {
            speed: ["auto", "10G", "1G", "100M", "10M"],
            vlanModes: ["access", "trunk"],
            flowCtl: ["none", "tx", "rx", "tx_rx"],
            confQueues: [0, 1, 2, 3, 4, 5, 6, 7],
            availableports: null
        }
    };

    (function init() {
        var promise = $http.get('/availableports');
        promise.then(function (response) {
            $scope.data.options.availableports = response.data.ports;
        });
        $scope.data.ports.findIndex = function(callback) {
            for (var i = 0; i < this.length; i++) {
                if (callback(this[i], i)) {
                    return i;
                }
            };
            return -1;
        };
    })();

    $scope.initializer = {
        newPort: function () {
                     var port = {vlan_config:{}, options:{}};

                     return {
                         init: function () {
                                   port.name = null,
                                   port.vlan_config.vlan_mode = "access",
                                   port.vlan_config.tag = 1,
                                   port.vlan_config.trunks = [],
                                   port.options.flow_ctl = "none"
                                   return port;
                               }
                     }
                 }
    };

    $scope.controlMethod = {
        edit: function (port) {
                  var myport = angular.copy(port);
                  if (myport.vlan_config.trunks === null) {
                      myport.vlan_config.trunks = [];
                  }
                  if (typeof myport.vlan_config.trunks === 'number') {
                      myport.vlan_config.trunks = [myport.vlan_config.trunks]
                  }
                  return myport;
              },
        isLagMember: function (options) {
                         return options["lacp-port-id"] !== null ||
                             options["lacp-port-priority"] != null ||
                             options["lacp-aggregation-key"] != null;
                     },
        overrideCb: function (port, editing) {
                        if (editing.qos.queues === null) {
                            if (port.qos.queues !== null) {
                                editing.qos.queues = angular.copy(port.qos.queues);
                            } else {
                                editing.qos.queues = [];
                            }
                        } else {
                            editing.qos.queues = null;
                        }
                    },
        uncheckLacpMember: function (port, editing) {
                               if (editing.options["lacp-port-id"] !== null ||
                             editing.options["lacp-port-priority"] !== null ||
                             editing.options["lacp-aggregation-key"] !== null) {
                                   editing.options["lacp-port-id"] = null;
                                   editing.options["lacp-port-priority"] = null;
                                   editing.options["lacp-aggregation-key"] = null;
                               } else {
                                   editing.options["lacp-port-id"] = port.options["lacp-port-id"];
                                   editing.options["lacp-port-priority"] = port.options["lacp-port-priority"];
                                   editing.options["lacp-aggregation-key"] = port.options["lacp-aggregation-key"];
                               }
                           },
        validOfPort : function (port) {
            return (port.ofport > 0) ? true : false;
        },
        displayLinkSpeed: function (speed) {
            return speed == null ? "Auto" : speed;
        }
    };

    $scope.crud = {
        create: function (record) {
                    record.name = record.name.name;
                    var promise = $http.post('/bridges/'+$scope.br.name+'/ports/'+record.name.split("/")[2]+'/add', record);
                    var success = {type: "success", msg: "A new port is being created. Please wait until it appears below."};
                    $scope.alerts.push(success);
                    promise.then(function(response) {
                        if (response.data.ret === 0) {
                            $scope.data.ports.push(response.data.port);
                            // $scope.br.ports = $scope.data.ports;
                        } else {
                            var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                            $scope.alerts.push(failure);
                        }
                    });
                },
        read: null,
        update: function (record) {
                    var promise = $http.post('/bridges/'+$scope.br.name+'/ports/'+record.name.split("/")[2]+'/update', record);
                    var success = {type: "success", msg: "Changes are being applied. Please wait before they take effect."};
                    $scope.alerts.push(success);
                    promise.then(function(response) {
                        if (response.data.ret === 0) {
                            var index = $scope.data.ports.findIndex($scope.port(record.name));
                            $scope.data.ports.splice(index, 1, response.data.port);
                            // $scope.br.ports = $scope.data.ports;
                        } else {
                            var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                            $scope.alerts.push(failure);
                        }
                    });
                },
        del: function (record) {
                 var promise = $http.post('/bridges/'+$scope.br.name+'/ports/'+record.name.split("/")[2]+'/del', record);
                 var success = {type: "success", msg: "A port is being removed. Please wait until it disappears from the list below."};
                 $scope.alerts.push(success);
                 promise.then(function(response) {
                     if (response.data.ret === 0) {
                         // The data refresh has to be delayed for 300 miliseconds. Otherwise,
                         // the scrollbar disappears in Chrome. But works fine in Firefox.
                         $timeout(function () {
                             var index = $scope.data.ports.findIndex($scope.port(record.name));
                             $scope.data.ports.splice(index, 1);
                             // $scope.br.ports = $scope.data.ports;
                         }, 300);
                     } else {
                         var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                         $scope.alerts.push(failure);
                     }
                 });
             }
    };
  }])
  .controller('MeterCtrl', ['$scope', '$http', function ($scope, $http) {
    var promise = $http.get('/bridges/'+$scope.br.name+'/meters');
    $scope.commonMethods.showSpinner();
    promise.then(function(response) {
        $scope.meters = response.data.meters;
        $scope.commonMethods.hideSpinner();
    });

    $scope.convertFlags = function (flags) {
        var res = [];
        for (var property in flags) {
            if (flags.hasOwnProperty(property)) {
                var tmp = {};
                tmp.desc = property;
                tmp.isSelected = flags[property];
                res.push(tmp);
            }
        }
        return res;
    };

    $scope.saveFlags = function (localFlag) {
        var res = {};
        for (var i = 0; i < localFlag.length; i++) {
            res[localFlag[i].desc] = localFlag[i].isSelected;
        }
        return res;
    };

    $scope.notDSCP = function (type) {
        return !(type === "DSCP_REMARK")
    };

    $scope.bandsEmpty = function (bands) {
        if (bands) {
            return (bands.length === 0)
        } else {
            return false;
        }
    };

    $scope.noBurst = function (localFlag) {
        if (localFlag) {
            for (var i = 0; i < localFlag.length; i++) {
                if (localFlag[i].desc === "BURST") {
                    return !localFlag[i].isSelected;
                }
            }
        } else {
            return false;
        }
    };

    $scope.band_types = ['DSCP_REMARK', 'DROP'];
    $scope.newMeter = function () {
        var meter = {flags:{},bands:[]};

        return {
            init: function () {
                      meter.id = null;
                      meter.flags.STATS = false;
                      meter.flags.KBPS = true;
                      meter.flags.BURST = false;
                      return meter;
                  }
        }
    };

    $scope.update_meter= function(record) {
        var promise = $http.post('/bridges/'+$scope.br.name+'/meters/'+record.id+'/update', record);
        promise.then(function(response) {
            if (response.data.ret === 0) {
                var success = {type: "success", msg: "Successfully updated."};
                $scope.meters = response.data.meters;
                $scope.br.meters = $scope.meters;
                $scope.alerts.push(success);
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }
        });
    };

    $scope.del_meter = function(record) {
        var promise = $http.post('/bridges/'+$scope.br.name+'/meters/'+record.id+'/del', record);
        promise.then(function(response) {
            if (response.data.ret === 0) {
                var success = {type: "success", msg: "Meter is removed."};
                $scope.meters = response.data.meters;
                $scope.br.meters = $scope.meters;
                $scope.alerts.push(success);
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }
        });
    };

    $scope.add_meter = function(record) {
        var promise = $http.post('/bridges/'+$scope.br.name+'/meters/'+record.id+'/add', record);
        promise.then(function(response) {
            if (response.data.ret === 0) {
                var success = {type: "success", msg: "A new meter has been added."};
                $scope.meters = response.data.meters;
                $scope.br.meters = $scope.meters;
                $scope.alerts.push(success);
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }
        });
    };

    $scope.clear = function(unused) {
        var promise = $http.delete('/bridges/'+$scope.br.name+'/meters');
        promise.then(function(response) {
            if (response.data.ret === 0) {
                var success = {type: "success", msg: "All meters have been removed."};
                $scope.meters = response.data.meters;
                $scope.br.meters = $scope.meters;
                $scope.alerts.push(success);
            } else {
                var failure = {type: "danger", msg: "Oops! Check things up and try again."};
                $scope.alerts.push(failure);
            }

        });
    };
  }]);
  //.controller('', [function () {
  //}]);
