'use strict';

angular.module('myApp.directives', []).
  directive('ipAddr', function() {
      var seg = '([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])';
      var IPv4_REGEXP = new RegExp("^("+seg+"\\.){3}"+seg+"$");
      return {
          require: 'ngModel',
          restrict: 'A',
          link: function (scope, elm, attrs, ctrl) {
              ctrl.$parsers.unshift(function(viewValue) {
                  if (IPv4_REGEXP.test(viewValue)) {
                      // it is valid
                      ctrl.$setValidity('ipAddr', true);
                      return viewValue;
                  } else {
                      // it is invalid, return undefined
                      ctrl.$setValidity('ipAddr', false);
                      return undefined;
                  }
              });
          }
      };
  }).
  directive('macAddr', function() {
      var seg = '[0-9a-f]{2}';
      var MAC_REGEXP = new RegExp("^"+seg+"(:"+seg+"){5}$");
      return {
          require: 'ngModel',
          restrict: 'A',
          link: function (scope, elm, attrs, ctrl) {
              ctrl.$parsers.unshift(function(viewValue) {
                  if (MAC_REGEXP.test(viewValue)) {
                      // it is valid
                      ctrl.$setValidity('macAddr', true);
                      return viewValue;
                  } else {
                      // it is invalid, return undefined
                      ctrl.$setValidity('macAddr', false);
                      return undefined;
                  }
              });
          }
      };
  }).
  directive('selectAll', function () {
      return {
          replace: true, // replace the current element
          restrict: "E", // Element name: <selectAll></selectAll>
          scope: {
              checkboxes: '=',
          },
          template: '<div class="checkbox">' +
                    '<label>' +
                    '<input type="checkbox" ng-model="master" ng-change="masterChange()">' +
                    '<div ng-transclude></div>' +
                    '</label>' +
                    '</div>',
          transclude: true,
          controller: function ($scope, $element) {
              $scope.masterChange = function () {
                  if ($scope.master) {
                      angular.forEach($scope.checkboxes, function (ele, ind) {
                          ele.isSelected = true;
                      });
                  } else {
                      angular.forEach($scope.checkboxes, function (ele, ind) {
                          ele.isSelected = false;
                      });
                  }
              };

              $scope.$watch('checkboxes', function () {
                  var allSet = true,
                      allClear = true;

                  angular.forEach($scope.checkboxes, function (ele, ind) {
                      if (ele.isSelected) {
                          allClear = false;
                      } else {
                          allSet = false;
                      }
                  });

                  //$element.prop('indeterminate', false);
                  if (allSet) {
                      $scope.master = true;
                  } else if (allClear) {
                      $scope.master = false;
                  } else {
                      $scope.master = false;
                      //$element.prop('indeterminate', true);
                  }
              }, true); // objectEquality: Compare object for equality rather than for reference.
          }
      }
  }).
  directive('nameUnique', function () {
      return {
          require: 'ngModel',
          restrict: "A",
          link: function (scope, elm, attrs, ctrl) {
              ctrl.$parsers.unshift(function (viewValue) {
                  var existingBr = [];
                  for (var i = 0; i < scope.bridges.length; i++) {
                      existingBr.push(scope.bridges[i].name);
                  }
                  if (existingBr.indexOf(viewValue) > -1 ) {
                      ctrl.$setValidity('nameUnique', false);
                      return undefined;
                  } else {
                      ctrl.$setValidity('nameUnique', true);
                      return viewValue;
                  }
              });
          }
      }
  }).
  directive('scrollSpy', ['$window', function ($window) {
      return {
          restrict: "A",
          controller: function ($scope) {
              $scope.spies = [];
              this.addSpy = function (spyObj) {
                  $scope.spies.push(spyObj);
              };
          },
          link: function (scope, elem, attrs) {
              var spyElems = [];

              scope.$watch('spies', function (spies) {
                  var len = spies.length;
                  for (var i = 0; i < len; i++) {
                      var spy = spies[i];
                      if (spyElems[spy.id] === null || spyElems[spy.id] === undefined) {
                          spyElems[spy.id] = angular.element(document.querySelector('#'+spy.id));
                      }
                  }
              });

              $($window).on('scroll', function () {
                  var highlightSpy = null;
                  var len = scope.spies.length;
                  for (var i = 0; i < len; i++) {
                      var spy = scope.spies[i];
                      spy.out();
                      var pos = jQuery(spyElems[spy.id]).offset().top - $window.scrollY;
                      if (pos <= 0) {
                          spy.pos = pos;
                          if (highlightSpy === null) {
                              highlightSpy = spy;
                          }
                          if (highlightSpy.pos < spy.pos) {
                              highlightSpy = spy;
                          }
                      }
                  }
                  if (highlightSpy !== null) {
                      highlightSpy.in();
                  }
              });
          }
      }
  }]).
  directive('myspy', function () {
      return {
          restrict: 'A',
          require: '^scrollSpy',
          link: function (scope, elem, attrs, ctrl) {
              ctrl.addSpy(
                  {
                      id: attrs.myspy,
                      in: function () {
                          elem.addClass('current');
                      },
                      out: function () {
                          elem.removeClass('current');
                      }
                  });
          }
      }
  }).
  directive('navUl', function () {
      return {
          restrict: 'E',
          replace: true,
          scope: {
              startTab: '@',
              tabs: '=allTabs',
              onSwitchView: '&onSelect',
              id: '@',
              class: '@'
          },
          template: '<ul id="id" class="{{class}}">' +
                    '<li class="list-group-item" ng-class="{current: isActive($index)}" ng-repeat="tab in tabs">' +
                    '<a href="" ng-click="selectPage($index)">{{tab.name}}</a>' +
                    '</li></ul>',
          link: function (scope) {
              scope.currentTab = scope.startTab;
              scope.isActive = function (index) {
                  return scope.currentTab === index;
              };
              scope.selectPage = function (index) {
                  if (!scope.isActive(index)) {
                      scope.currentTab = index;
                      scope.onSwitchView({index: index});
                  }
              };
          },
          controller: ['$scope', function($scope) {
                          $scope.$on('displayBridge', function(event, data) {
                              $scope.selectPage(data);
                          });
                      }]
      }
  }).
  directive('bridgeLists', function () {
      return {
          restrict: 'E',
          replace: true,
          scope: {
              bridges: '=logicBridges',
              onSelect: '&',
              monitor: '&'
          },
          template: '<ul class="list-group">' +
                    '<li class="list-group-item" ng-class="{current: isActive(0)}">' +
                    '<a href="" ng-click="whichBridge(0); monitor({brName: \'phy switch\'})">Switch Resource</a>' +
                    '</li>' +
                    '<li class="list-group-item" ng-class="{current: isActive($index+1)}" ng-repeat="bridge in bridges | orderBy:'+"'+name'\">" +
                    '<a href="" ng-click="whichBridge($index+1); monitor({brName: bridge.name})">{{bridge.name}}</a>' +
                    '</li></ul>',
          link: function (scope) {
              scope.current = 0;
              scope.hardwareSwitch = {name: 'phy switch'};

              scope.isActive = function (index) {
                  return scope.current === index;
              };

              scope.whichBridge = function (index) {
                  if (!scope.isActive(index)) {
                      scope.current = index;
                      if (index === 0) {
                          scope.onSelect({index: index});
                      } else {
                          // Use the same page to display bridge info
                          scope.onSelect({index: 1});
                      }
                  }
              };
          }
      }
  }).
  directive('confirmBeforeRemoval', function () {
      return {
          restrict: 'E',
          replace: true,
          scope: {
              class: '@',
              ref: '@'
          },
          template: '<a class="{{button}}" data-toggle="modal" href="{{ref}}">Delete</a>',
          link: function (scope) {
          }
      }
  }).
  directive('removeItemWindow', function () {
      return {
          restrict: 'E',
          replace: true,
          transclude: true,
          scope: {
              callback: '&',
              refId: '@',
              object: '=item',
              afterRemoval: '&'
          },
          template: '<div class="modal fade" id="{{refId}}" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="true" >' +
                        '<div class="modal-dialog">' +
                            '<div class="modal-content">' +
                                '<div class="modal-header">' +
                                    '<button type="button" class="close" data-dismiss="modal" aria-hidden="true">&times;</button>' +
                                    '<h4 class="modal-title">Confirmation</h4>' +
                                '</div>' +
                                '<div class="modal-body" ng-transclude>' +
                                '</div>' +
                                '<div class="modal-footer">' +
                                    '<button type="button" class="btn btn-default" data-dismiss="modal">Cancel</button>' +
                                    '<button type="button" class="btn btn-default" data-dismiss="modal" ng-click="callback({item: object}); afterRemoval()">Delete</button>' +
                                '</div>' +
                            '</div>' +
                        '</div>' +
                    '</div>'
      }
  }).
  directive('breadcrumbs', function () {
      return {
          restrict: 'E',
          replace: true,
          scope: {
              hierarchy: '='
          },
          template: '<ol class="breadcrumb">' +
                        '<li ng-class="{active: $last}" ng-repeat="level in hierarchy">{{level}}' +
                    '</ol>'
      }
  }).
  directive('matchFields', function () {
      return {
          require: 'ngModel',
          restrict: 'A',
          link: function (scope, elm, attrs, ctrl) {
              ctrl.$parsers.push(function(viewValue) {
                  var validity = validMatchFields(viewValue);

                  if (validity.isValid) {
                      ctrl.$setValidity('matchFields', true);
                      scope.controlMethod.setFlowDescValidity("Match fields description is valid.");
                      return viewValue;
                  } else {
                      ctrl.$setValidity('matchFields', false);
                      scope.controlMethod.setFlowDescValidity("Error in: '" + validity.invalidSection + "'. Reason: '" + validity.invalidReason + "'");
                      return viewValue;
                  }
              });
              ctrl.$formatters.push(function(modelValue) {
                  if (!modelValue) {
                      ctrl.$setValidity('matchFields', true);
                      scope.controlMethod.setFlowDescValidity("Match field description is valid.");
                      return modelValue;
                  }

                  var validity = validMatchFields(modelValue);

                  if (validity.isValid) {
                      ctrl.$setValidity('matchFields', true);
                      scope.controlMethod.setFlowDescValidity("Match field description is valid.");
                      return modelValue;
                  } else {
                      ctrl.$setValidity('matchFields', false);
                      scope.controlMethod.setFlowDescValidity("Error in: '" + validity.invalidSection + "'. Reason: '" + validity.invalidReason + "'");
                      return modelValue;
                  }
              });
          }
      };

      function validMatchFields(input) {
          // Return as soon as an invalid field description is encountered.
          // The object to return:
          var validity = {
              isValid: true,
              invalidSection: null,
              invalidReason: null
          };

          if (input) {
              var fields = input.split(',');

              for (var i in fields) {
                  // Fields description ends with ','. so this 'if' clause is checking if we've 
                  // reached the end of the fileds description.
                  if (fields[i] !== '') {
                      var fieldEntry = fields[i].split('=')
                      if (fieldEntry.length !== 2) {
                          // TODO shorthand notations
                          var shorthandNotation = ['ip', 'icmp', 'tcp', 'udp', 'arp', 'rarp', 'send_flow_rem'];
                          if (shorthandNotation.indexOf(fieldEntry[0]) < 0) {
                              validity.isValid = false;
                              validity.invalidSection = fields[i];
                              validity.invalidReason = "invalid shorthand notation."
                              return validity;
                          }
                      } else {
                          var fieldName = fieldEntry[0];
                          var fieldValue = fieldEntry[1];
                          var perFieldRes = (validator(fieldName))(fieldValue);
                          if (!perFieldRes.isValid) {
                              validity.isValid = false;
                              validity.invalidSection = fields[i];
                              validity.invalidReason = perFieldRes.invalidReason;
                              return validity;
                          }
                      }
                  }
              }
              // All fields are specified correctly.
              // TODO: extra policy
              return validity
          } else {
              return validity;
          }
      }

      function validator(fieldName) {
          // If fieldName is supported, return a specific validator.
          var result = {
              isValid: {
                  isValid: true,
                  invalidReason: null
              },
              isInvalidWithReason: function (reason) {
                  return {
                      isValid: false,
                      invalidReason: reason
                  }
              }
          };

          function intRange(val, min, max) {
              return val >= min && val <= max;
          }

          function macAddr(val) {
              var seg = '[0-9a-f]{2}';
              var MAC_REGEXP = new RegExp("^"+seg+"(:"+seg+"){5}$");
              return MAC_REGEXP.test(val);
          }

          function ipv4Addr(val) {
              var seg = '([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])';
              var IPv4_REGEXP = new RegExp("^("+seg+"\\.){3}"+seg+"$");
              return IPv4_REGEXP.test(val);
          }

          function ipv4NetMask(val) {
              if (ipv4Addr(val)) {
                  return true;
              } else {
                  var netmask = parseInt(val, 10);
                  if (isNaN(netmask)) {
                      return false;
                  } else {
                      return intRange(netmask, 0, 32)
                  }
              }
          }

          function ipv6Addr(val) {
              var reg = /^((?=.*::)(?!.*::.+::)(::)?([\dA-F]{1,4}:(:|\b)|){5}|([\dA-F]{1,4}:){6})((([\dA-F]{1,4}((?!\3)::|:\b|$))|(?!\2\3)){2}|(((2[0-4]|1\d|[1-9])?\d|25[0-5])\.?\b){4})$/i;
              return reg.test(val);
          }

          function ipv6NetMask(val) {
              if (ipv6Addr(val)) {
                  return true;
              } else {
                  var netmask = parseInt(val, 10);
                  if (isNaN(netmask)) {
                      return false;
                  } else {
                      return intRange(netmask, 0, 128)
                  }
              }
          }

          var visualCandyCheckers = [
              {
                  fieldName: 'in_port',
                  checker: function (val) {
                      var in_port = parseInt(val);
                      if (!isNaN(in_port) || val === 'controller' || val === 'local')
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("IN_PORT should be a positive number or one of the reserved ports.");
                  }
              },
              {
                  fieldName: 'dl_vlan',
                  checker: function (val) {
                      var dl_type = parseInt(val);
                      if (!isNaN(dl_type) && intRange(dl_type, 1, 4094))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("DL_VLAN should be a number between 1 and 4094, inclusive.");
                  }
              },
              {
                  fieldName: 'dl_vlan_pcp',
                  checker: function (val) {
                      var dl_vlan_pcp = parseInt(val);
                      if (!isNaN(dl_vlan_pcp) && intRange(dl_vlan_pcp, 0, 7))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("DL_VLAN_PCP should be a number between 0 and 7, inclusive.");
                  }
              },
              {
                  fieldName: 'dl_src',
                  checker: function (val) {
                      if (macAddr(val))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("invalid MAC address.");
                  }
              },
              {
                  fieldName: 'dl_dst',
                  checker: function (val) {
                      if (macAddr(val))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("invalid MAC address.");
                  }
              },
              {
                  fieldName: 'dl_type',
                  checker: function (val) {
                      // if val is a hexadecimal number, convert it to decimal number
                      if (/0x.*/.test(val)) {
                          val = parseInt(val, 16).toString();
                      }
                      var dl_type = parseInt(val);
                      if (!isNaN(dl_type) && intRange(dl_type, 0, 65535))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("DL_TYPE should be a number between 0 and 65535, inclusive.");
                  }
              },
              {
                  fieldName: 'arp_sha',
                  checker: function (val) {
                      if (macAddr(val))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("invalid hardware address.");
                  }
              },
              {
                  fieldName: 'arp_tha',
                  checker: function (val) {
                      if (macAddr(val))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("invalid hardware address.");
                  }
              },
              {
                  fieldName: 'mpls_tc',
                  checker: function (val) {
                      var dl_vlan_pcp = parseInt(val);
                      if (!isNaN(dl_vlan_pcp) && intRange(dl_vlan_pcp, 0, 7))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("MPLS_TC should be a number between 0 and 7, inclusive.");
                  }
              },
              {
                  fieldName: 'mpls_label',
                  checker: function (val) {
                      // if val is a hexadecimal number, convert it to decimal number
                      if (/0x.*/.test(val)) {
                          val = parseInt(val, 16).toString();
                      }
                      var dl_type = parseInt(val);
                      if (!isNaN(dl_type) && intRange(dl_type, 0, 1048575))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("MPLS_LABEL should be a number between 0 and 1048575, inclusive.");
                  }
              },
              {
                  fieldName: 'nw_src',
                  checker: function (val) {
                      if (val.indexOf('/') < 0) {
                          if (ipv4Addr(val)) {
                              return result.isValid;
                          } else {
                              return result.isInvalidWithReason("invalid IPv4 address.");
                          }
                      } else {
                          var arrayOfAddr = val.split('/');
                          var address = arrayOfAddr[0];
                          var netmask = arrayOfAddr[1];
                          if (ipv4Addr(address) && ipv4NetMask(netmask)) {
                              return result.isValid;
                          } else {
                              return result.isInvalidWithReason("invalid IPv4 address.");
                          }
                      }
                  }
              },
              {
                  fieldName: 'nw_dst',
                  checker: function (val) {
                      if (val.indexOf('/') < 0) {
                          if (ipv4Addr(val)) {
                              return result.isValid;
                          } else {
                              return result.isInvalidWithReason("invalid IPv4 address.");
                          }
                      } else {
                          var arrayOfAddr = val.split('/');
                          var address = arrayOfAddr[0];
                          var netmask = arrayOfAddr[1];
                          if (ipv4Addr(address) && ipv4NetMask(netmask)) {
                              return result.isValid;
                          } else {
                              return result.isInvalidWithReason("invalid IPv4 address.");
                          }
                      }
                  }
              },
              {
                  fieldName: 'ipv6_src',
                  checker: function (val) {
                      if (val.indexOf('/') < 0) {
                          if (ipv6Addr(val)) {
                              return result.isValid;
                          } else {
                              return result.isInvalidWithReason("invalid IPv6 address.");
                          }
                      } else {
                          var arrayOfAddr = val.split('/');
                          var address = arrayOfAddr[0];
                          var netmask = arrayOfAddr[1];
                          if (ipv6Addr(address) && ipv6NetMask(netmask)) {
                              return result.isValid;
                          } else {
                              return result.isInvalidWithReason("invalid IPv6 address.");
                          }
                      }
                  }
              },
              {
                  fieldName: 'ipv6_dst',
                  checker: function (val) {
                      if (val.indexOf('/') < 0) {
                          if (ipv6Addr(val)) {
                              return result.isValid;
                          } else {
                              return result.isInvalidWithReason("invalid IPv6 address.");
                          }
                      } else {
                          var arrayOfAddr = val.split('/');
                          var address = arrayOfAddr[0];
                          var netmask = arrayOfAddr[1];
                          if (ipv6Addr(address) && ipv6NetMask(netmask)) {
                              return result.isValid;
                          } else {
                              return result.isInvalidWithReason("invalid IPv6 address.");
                          }
                      }
                  }
              },
              {
                  fieldName: 'nw_proto',
                  checker: function (val) {
                      var nw_proto = parseInt(val);
                      if (!isNaN(nw_proto) && intRange(nw_proto, 0, 255))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("NW_PROTO should be a number between 0 and 255, inclusive.");
                  }
              },
              {
                  fieldName: 'nw_tos',
                  checker: function (val) {
                      var nw_tos = parseInt(val);
                      if (!isNaN(nw_tos) && intRange(nw_tos, 0, 255))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("NW_TOS should be a number between 0 and 255, inclusive.");
                  }
              },
              {
                  fieldName: 'nw_ecn',
                  checker: function (val) {
                      var nw_ecn = parseInt(val);
                      if (!isNaN(nw_ecn) && intRange(nw_ecn, 0, 3))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("NW_ECN should be a number between 0 and 3, inclusive.");
                  }
              },
              /*
               * {
               *     fieldName: 'nw_ttl',
               *     checker: function (val) {
               *         var nw_ttl = parseInt(val);
               *         if (!isNaN(nw_ttl) && intRange(nw_ttl, 0, 255))
               *             return result.isValid;
               *         else
               *             return result.isInvalidWithReason("NW_TTL should be a number between 0 and 255, inclusive.");
               *     }
               * },
               */
              {
                  fieldName: 'tp_src',
                  checker: function (val) {
                      var tp_src = parseInt(val);
                      if (!isNaN(tp_src) && intRange(tp_src, 0, 65535))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("TP_SRC should be a number between 0 and 65535, inclusive.");
                  }
              },
              {
                  fieldName: 'tp_dst',
                  checker: function (val) {
                      var tp_dst = parseInt(val);
                      if (!isNaN(tp_dst) && intRange(tp_dst, 0, 65535))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("TP_DST should be a number between 0 and 65535, inclusive.");
                  }
              },
              {
                  fieldName: 'icmp_type',
                  checker: function (val) {
                      var icmp_type = parseInt(val);
                      if (!isNaN(icmp_type) && intRange(icmp_type, 0, 255))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("ICMP_TYPE should be a number between 0 and 255, inclusive.");
                  }
              },
              {
                  fieldName: 'icmp_code',
                  checker: function (val) {
                      var icmp_code = parseInt(val);
                      if (!isNaN(icmp_code) && intRange(icmp_code, 0, 255))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("ICMP_CODE should be a number between 0 and 255, inclusive.");
                  }
              },
              {
                  fieldName: 'idle_timeout',
                  checker: function (val) {
                      var timeout = parseInt(val);
                      if (!isNaN(timeout) && intRange(timeout, 0, Number.MAX_VALUE))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("IDLE_TIMEOUT should be a non-negative number.");
                  }
              },
              {
                  fieldName: 'hard_timeout',
                  checker: function (val) {
                      var timeout = parseInt(val);
                      if (!isNaN(timeout) && intRange(timeout, 0, Number.MAX_VALUE))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("HARD_TIMEOUT should be a non-negative number.");
                  }
              },
              {
                  fieldName: 'priority',
                  checker: function (val) {
                      var priority = parseInt(val);
                      if (!isNaN(priority) && intRange(priority, 0, 65535))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("PRIORITY should be a number between 0 and 65535.");
                  }
              },
              {
                  fieldName: 'cookie',
                  checker: function (val) {
                      var cookie = parseInt(val);
                      if (!isNaN(cookie))
                          return result.isValid;
                      else
                          return result.isInvalidWithReason("COOKIE should be a 64-bit number.");
                  }
              }
          ];

          var fieldChecker = {};
          for (var i in visualCandyCheckers) {
              fieldChecker[visualCandyCheckers[i].fieldName] = visualCandyCheckers[i].checker;
          }

          if (fieldName in fieldChecker) {
              return fieldChecker[fieldName];
          } else {
              return function (val) {
                  return {
                      isValid: false,
                      invalidReason: fieldName + " is not a valid field name."
                  }
              }
          };
      }
  }).
  directive('flowActions', function () {
        return {
          require: 'ngModel',
          restrict: 'A',
          link: function (scope, elm, attrs, ctrl) {
              ctrl.$parsers.push(function(viewValue) {
                  var validity = validActionsList(viewValue);

                  if (validity.isValid) {
                      ctrl.$setValidity('flowActions', true);
                      scope.controlMethod.setActionsDescValidity("List of actions is valid.");
                      return viewValue;
                  } else {
                      ctrl.$setValidity('flowActions', false);
                      scope.controlMethod.setActionsDescValidity("Error in: '" + validity.invalidSection + "'. Reason: '" + validity.invalidReason + "'");
                      return viewValue;
                  }
              });
              ctrl.$formatters.push(function(modelValue) {
                  if (!modelValue) {
                      ctrl.$setValidity('flowActions', true);
                      scope.controlMethod.setActionsDescValidity("List of actions is valid.");
                      return modelValue;
                  }

                  var validity = validActionsList(modelValue);

                  if (validity.isValid) {
                      ctrl.$setValidity('flowActions', true);
                      scope.controlMethod.setActionsDescValidity("List of actions is valid.");
                      return modelValue;
                  } else {
                      ctrl.$setValidity('flowActions', false);
                      scope.controlMethod.setActionsDescValidity("Error in: '" + validity.invalidSection + "'. Reason: '" + validity.invalidReason + "'");
                      return modelValue;
                  }
              });
          }
      };

      function validActionsList(input) {
          // Return as soon as an invalid action is encountered.
          // The object to return:
          var validity = {
              isValid: true,
              invalidSection: null,
              invalidReason: null
          };

          if (input) {
              var actions = input.split(',');

              for (var i in actions) {
                  if (actions[i] !== '') {
                      var actionEntry = ifActionIsSupported(actions[i])
                      if (actionEntry.action === null) {
                          validity.isValid = false;
                          validity.invalidSection = actions[i];
                          validity.invalidReason = "Unknown action.";
                          return validity;
                      } else {
                          var actionName = actionEntry.action;
                          var param = actionEntry.parameter;
                          if (actionName.toLowerCase() === 'controller') {
                              // Special case for controller(max_len=100)
                              var controllerRe = /controller(\(.*\))?,|controller(\(.*\))?$/i.exec(input);
                              if (controllerRe === null) {
                                  validity.isValid = false;
                                  validity.invalidSection = actions[i];
                                  validity.invalidReason = "Syntax error. Specify CONTROLLER as a target in this format controller[(key=value...)], where the valid keys are max_len, reason and id.";
                                  return validity;
                              } else {
                                  param = controllerRe[1] ? controllerRe[1] : controllerRe[2];
                                  if (param === undefined)
                                      param = '';
                              }
                          } else if (actionName.toLowerCase() === 'resubmit') {
                              // Special case for resubmit:3 or resubmit(3, 7)
                              var resubmitRe = /resubmit(:\d+|\(\d+,\d+\)),|resubmit(:\d+|\(\d+,\d+\))$/i.exec(input);
                              if (resubmitRe === null) {
                                  validity.isValid = false;
                                  validity.invalidSection = actions[i];
                                  validity.invalidReason = "Syntax error. The correct form for RESUBMIT action is either resubmit:port or resubmit(port,table).";
                                  return validity;
                              } else {
                                  param = resubmitRe[1] ? resubmitRe[1] : resubmitRe[2];
                              }
                          }
                          var perActionRes = (validator(actionName.toLowerCase()))(param.toLowerCase());
                          if (!perActionRes.isValid) {
                              validity.isValid = false;
                              validity.invalidSection = actions[i];
                              validity.invalidReason = perActionRes.invalidReason;
                              return validity;
                          }
                      }
                  }
              }
              // All fields are specified correctly.
              // TODO: extra policy
              return validity
          } else {
              return validity;
          }
      }

      function ifActionIsSupported(target) {
          var supportedActions = /^(output|enqueue|normal|flood|all|controller|drop|mod_vlan_vid|mod_vlan_pcp|strip_vlan|mod_dl_src|mod_dl_dst|push_vlan|push_mpls|pop_mpls|resubmit|set_field|mod_nw_src|mod_nw_dst|mod_nw_tos|mod_tp_src|mod_tp_dst|group|meter|goto_table|set_mpls_ttl|set_queue|push_pbb|pop_pbb|pop_vlan|max_len|reason|id|\d+)/i;
          var result = supportedActions.exec(target);
          if (result) {
              return {
                  action: result[0],
                  parameter: result.input.slice(result[0].length)
              }
          } else {
              return {
                  action: null,
                  parameter: target
              }
          }
      }

      function validator(actionName) {
          // If actionName is supported, return a specific validator.
          var result = {
              isValid: {
                  isValid: true,
                  invalidReason: null
              },
              isInvalidWithReason: function (reason) {
                  return {
                      isValid: false,
                      invalidReason: reason
                  }
              }
          };

          function intRange(val, min, max) {
              return val >= min && val <= max;
          }

          var visualCandyCheckers = [
              {
                  actionName: 'output',
                  checker: function (val) {
                      if (!val || val[0] !== ':') {
                          return result.isInvalidWithReason("Syntax error.");
                      } else {
                          val = val.slice(1);
                          var param = parseInt(val);
                          if (!isNaN(param) || val === 'in_port' || val === 'local')
                              return result.isValid;
                          else
                              return result.isInvalidWithReason("OpenFlow port number should be a positive number or one of the reserved ports.");
                      }
                  }
              },
              {
                  actionName: 'controller',
                  checker: function (val) {
                      function isControllerParamValid(param) {
                          var re = /\((.*)\)/.exec(param)
                          if (re === null) {
                              return false;
                          } else {
                              var keys = ['max_len', 'reason', 'id'];
                              var reasons = ['action', 'no_match', 'invalid_ttl'];
                              var pairs = re[1].split(',');
                              for (var i in pairs) {
                                  var pair = pairs[i];
                                  var tmp = pair.split('=');
                                  var key = tmp[0];
                                  var value = tmp[1];
                                  if (keys.indexOf(key) < 0)
                                      return false;
                                  if (key === 'reason') {
                                      if (reasons.indexOf(value) < 0)
                                          return false;
                                  }
                              }
                          }
                          return true;
                      }

                      if (!val) {
                          return result.isValid;
                      } else {
                          if (isControllerParamValid(val)) {
                              return result.isValid;
                          } else {
                              return result.isInvalidWithReason("Syntax error. Specify controller as a target in this format controller[(key=value...)], where the valid keys are max_len, reason and id.");
                          }
                      }
                  }
              },
              {
                  actionName: 'resubmit',
                  checker: function (val) {
                      var re = /:\d+|\(\d+,\d+\)/;
                      return re.test(val) ? result.isValid : result.isInvalidWithReason("Syntax error. The correct form for RESUBMIT action is either resubmit:port or resubmit(port,table).");
                  }
              },
              {
                  actionName: 'normal',
                  checker: function (val) {
                      return (val === '') ? result.isValid : result.isInvalidWithReason("Syntax error. Specify NORMAL as target to subject the packet to the device’s normal L2/L3 processing.");
                  }
              },
              {
                  actionName: 'flood',
                  checker: function (val) {
                      return (val === '') ? result.isValid : result.isInvalidWithReason("Syntax error. Specify FLOOD as target to output the packet on all switch physical ports other than the port on which it was received and any ports on which flooding is disabled.");
                  }
              },
              {
                  actionName: 'all',
                  checker: function (val) {
                      return (val === '') ? result.isValid : result.isInvalidWithReason("Syntax error. Specify ALL as target to output the packet on all switch physical ports other than the port on which it was received.");
                  }
              },
              {
                  actionName: 'drop',
                  checker: function (val) {
                      return (val === '') ? result.isValid : result.isInvalidWithReason("Syntax error. Specify DROP as target to discard the packet, so no further processing or forwarding takes place. If a drop action is used, no other actions may be specified.");
                  }
              },
              {
                  actionName: 'strip_vlan',
                  checker: function (val) {
                      return (val === '') ? result.isValid : result.isInvalidWithReason("Syntax error. Specify STRIP_VLAN as target to strip the VLAN tag from a packet if it is present.");
                  }
              },
              {
                  actionName: 'pop_vlan',
                  checker: function (val) {
                      return (val === '') ? result.isValid : result.isInvalidWithReason("Syntax error. Specify POP_VLAN as target to strip the VLAN tag from a packet if it is present.");
                  }
              },
              {
                  actionName: 'push_pbb',
                  checker: function (val) {
                          return (/^:(0x88e7|0x88E7|34888)$/.test(val)) ? result.isValid : result.isInvalidWithReason("Syntax error. The correct form for PUSH_PBB action is push_pbb:ethertype, where only ethertype 0x0x88e7 should be used.");
                  }
              },
              {
                  actionName: 'pop_pbb',
                  checker: function (val) {
                      return (val === '') ? result.isValid : result.isInvalidWithReason("Syntax error. Specify POP_PBB as target to strip the PBB tag from a packet if it is present.");
                  }
              },
              {
                  actionName: 'enqueue',
                  checker: function (val) {
                      var re = /^:(\d+|local|in_port):(\d+)$/i;
                      var resArray = re.exec(val);
                      if (resArray === null)
                          return result.isInvalidWithReason("Syntax error. The correct form for ENQUEUE action is enqueue:port:queue.");
                      else {
                          var queueIndex = resArray[2];
                          if (intRange(parseInt(queueIndex), 0, 7))
                              return result.isValid;
                          else
                              return result.isInvalidWithReason("There are eight queues on each port, indexed from 0 to 7.");
                      }
                  }
              },
              {
                  actionName: 'mod_vlan_vid',
                  checker: function (val) {
                      var re = /^:(\d+)$/;
                      var resArray = re.exec(val);
                      if (resArray === null)
                          return result.isInvalidWithReason("Syntax error. The correct form for MOD_VLAN_VID action is mod_vlan_vid:vlan_vid.");
                      else {
                          var vid = resArray[1];
                          if (intRange(parseInt(vid), 1, 4094))
                              return result.isValid;
                          else
                              return result.isInvalidWithReason("VLAN ID should be between 1 and 4094.");
                      }
                  }
              },
              {
                  actionName: 'mod_vlan_pcp',
                  checker: function (val) {
                      var re = /^:(\d+)$/;
                      var resArray = re.exec(val);
                      if (resArray === null)
                          return result.isInvalidWithReason("Syntax error. The correct form for MOD_VLAN_PCP action is mod_vlan_pcp:vlan_pcp.");
                      else {
                          var vid = resArray[1];
                          if (intRange(parseInt(vid), 0, 7))
                              return result.isValid;
                          else
                              return result.isInvalidWithReason("VLAN PRIORITY should be between 0 and 7.");
                      }
                  }
              },
              {
                  actionName: 'mod_dl_src',
                  checker: function (val) {
                      var seg = '[0-9a-f]{2}';
                      var MAC_REGEXP = new RegExp("^:("+seg+"(?::"+seg+"){5})$");
                      var resArray = MAC_REGEXP.exec(val);
                      if (resArray === null)
                          return result.isInvalidWithReason("Syntax error. The correct form for MOD_DL_SRC action is mod_dl_src:mac.");
                      else
                          return result.isValid;
                  }
              },
              {
                  actionName: 'mod_dl_dst',
                  checker: function (val) {
                      var seg = '[0-9a-f]{2}';
                      var MAC_REGEXP = new RegExp("^:("+seg+"(?::"+seg+"){5})$");
                      var resArray = MAC_REGEXP.exec(val);
                      if (resArray === null)
                          return result.isInvalidWithReason("Syntax error. The correct form for MOD_DL_DST action is mod_dl_dst:mac.");
                      else
                          return result.isValid;
                  }
              },
              {
                  actionName: 'push_vlan',
                  checker: function (val) {
                          return (/^:0x8100|33024$/.test(val)) ? result.isValid : result.isInvalidWithReason("Syntax error. The correct form for PUSH_VLAN action is push_vlan:ethertype, where only ethertype 0x8100 should be used.");
                  }
              },
              {
                  actionName: 'push_mpls',
                  checker: function (val) {
                          return (/^:(0x8848|0x8847|34887|34888)$/.test(val)) ? result.isValid : result.isInvalidWithReason("Syntax error. The correct form for PUSH_MPLS action is push_mpls:ethertype, where either the MPLS unicast Ethertype 0x8847 or the MPLS multicast Ethertype 0x8848 should be used.");
                  }
              },
              {
                  actionName: 'pop_mpls',
                  checker: function (val) {
                          return (/^:(0x8848|0x8847|34887|34888)$/.test(val)) ? result.isValid : result.isInvalidWithReason("Syntax error. The correct form for POP_VLAN action is pop_vlan:ethertype, where either the MPLS unicast Ethertype 0x8847 or the MPLS multicast Ethertype 0x8848 should be used.");
                  }
              },
              {
                  actionName: 'set_mpls_ttl',
                  checker: function (val) {
                      var re = /^:\d+$/;
                      var resArray = re.exec(val);
                      if (resArray === null)
                          return result.isInvalidWithReason("Syntax error. The correct form for SET_MPLS_TTL action is set_mpls_ttl:ttl. ttl should be in the range 0 to 255 inclusive.");
                      else
                          return result.isValid;
                  }
              },
              {
                  actionName: 'set_queue',
                  checker: function (val) {
                      var re = /^:\d+$/;
                      var resArray = re.exec(val);
                      if (resArray === null)
                          return result.isInvalidWithReason("Syntax error. The correct form for SET_QUEUE action is set_queue:queue. queue should be in the range 0 to 7 inclusive.");
                      else
                          return result.isValid;
                  }
              },
              {
                  actionName: 'set_field',
                  checker: function (val) {
                      return  (/^:[^->]+->.[^->]+$/.test(val)) ? result.isValid : result.isInvalidWithReason("Syntax error. The correct form for SET_FIELD action is set_field:value->dst.");
                  }
              },
              {
                  actionName : 'mod_nw_src',
                  checker: function (val) {
                      var seg = '([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])';
                      var IPv4_REGEXP = new RegExp("^:("+seg+"\\.){3}"+seg+"$");
                      return (IPv4_REGEXP.test(val)) ? result.isValid : result.isInvalidWithReason("Syntax error. The correct form for MOD_NW_SRC action is mod_nw_src:ip.");
                  }
              },
              {
                  actionName : 'mod_nw_dst',
                  checker: function (val) {
                      var seg = '([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])';
                      var IPv4_REGEXP = new RegExp("^:("+seg+"\\.){3}"+seg+"$");
                      return (IPv4_REGEXP.test(val)) ? result.isValid : result.isInvalidWithReason("Syntax error. The correct form for MOD_NW_DST action is mod_nw_dst:ip.");
                  }
              },
              {
                  actionName: 'mod_nw_tos',
                  checker: function (val) {
                      var re = /^:(\d+)$/;
                      var resArray = re.exec(val);
                      if (resArray === null)
                          return result.isInvalidWithReason("Syntax error. The correct form for MOD_NW_TOS action is mod_nw_tos:tos.");
                      else {
                          var vid = resArray[1];
                          if (intRange(parseInt(vid), 0, 255))
                              return result.isValid;
                          else
                              return result.isInvalidWithReason("IPv4 ToS/DSCP must be a multiple of 4 between 0 and 255.");
                      }
                  }
              },
              {
                  actionName: 'mod_tp_src',
                  checker: function (val) {
                      var re = /^:(\d+)$/;
                      var resArray = re.exec(val);
                      if (resArray === null)
                          return result.isInvalidWithReason("Syntax error. The correct form for MOD_TP_SRC action is mod_tp_src:port.");
                      else {
                          var vid = resArray[1];
                          if (intRange(parseInt(vid), 0, 65535))
                              return result.isValid;
                          else
                              return result.isInvalidWithReason("TCP or UDP port number should be between 0 and 65535, inclusive.");
                      }

                  }
              },
              {
                  actionName: 'mod_tp_dst',
                  checker: function (val) {
                      var re = /^:(\d+)$/;
                      var resArray = re.exec(val);
                      if (resArray === null)
                          return result.isInvalidWithReason("Syntax error. The correct form for MOD_TP_DST action is mod_tp_dst:port.");
                      else {
                          var vid = resArray[1];
                          if (intRange(parseInt(vid), 0, 65535))
                              return result.isValid;
                          else
                              return result.isInvalidWithReason("TCP or UDP port number should be between 0 and 65535, inclusive.");
                      }

                  }
              },
              {
                  actionName: 'group',
                  checker: function (val) {
                      var re = /^:\d+$/;
                      var resArray = re.exec(val);
                      if (resArray === null) {
                          return result.isInvalidWithReason("Syntax error. The correct form for GROUP action is group:group_id.");
                      } else {
                          return result.isValid;
                      }
                  }
              },
              {
                  actionName: 'meter',
                  checker: function (val) {
                      var re = /^:\d+$/;
                      var resArray = re.exec(val);
                      if (resArray === null) {
                          return result.isInvalidWithReason("Syntax error. The correct form for METER action is meter:meter_id.");
                      } else {
                          return result.isValid;
                      }
                  }
              },
              {
                  actionName: 'goto_table',
                  checker: function (val) {
                      var re = /^:\d+$/;
                      var resArray = re.exec(val);
                      if (resArray === null) {
                          return result.isInvalidWithReason("Syntax error. The correct form for GOTO_TABLE action is goto_table:table_id.");
                      } else {
                          return result.isValid;
                      }
                  }
              }
          ];

          var actionChecker = {};
          for (var i in visualCandyCheckers) {
              actionChecker[visualCandyCheckers[i].actionName] = visualCandyCheckers[i].checker;
          }

          if (actionName in actionChecker) {
              return actionChecker[actionName];
          } else if ((['max_len', 'reason', 'id'].indexOf(actionName) >= 0) || /\d+/.test(actionName)) {
              return function (val) {
                  return {
                      isValid: true,
                      invalidReason: null
                  }
              }
          } else {
              return function (val) {
                  return {
                      isValid: false,
                      invalidReason: "Unknown action."
                  }
              }
          };
      }
});
