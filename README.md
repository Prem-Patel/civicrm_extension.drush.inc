civicrm_extension.drush.inc
===========================

Drush commands for CiviCRM extensions

Commands
--------
drush civicrm-extension-directory - list all extensions on the extensions directory
drush civicrm-extension-status - show the status of all locally installed / uninstalled extensions
drush civicrm-extension-download <key> - download the extension specified by <key>
drush civicrm-extension-enable <key> - enable the extension specified by <key>
drush civicrm-extension-disable <key> - disable the extension specified by <key>
drush civicrm-extension-uninstall <key> - uninstall the extension specified by <key>

To avoid file permission problems, run as the webserver user - eg: sudo -u www-data drush civicrm-extension-status