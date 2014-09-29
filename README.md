civicrm_extension.drush.inc
===========================

Drush commands for CiviCRM extensions

Commands
--------
* drush civicrm-extension-directory - list all extensions on the extensions directory
* drush civicrm-extension-status - show the status of all locally installed / uninstalled extensions
* drush civicrm-extension-download &lt;key&gt; - download the extension specified by &lt;key&gt;
* drush civicrm-extension-enable &lt;key&gt; - enable the extension specified by &lt;key&gt;
* drush civicrm-extension-disable &lt;key&gt; - disable the extension specified by &lt;key&gt;
* drush civicrm-extension-uninstall &lt;key&gt; - uninstall the extension specified by &lt;key&gt;

To avoid file permission problems, run as the webserver user - eg: sudo -u www-data drush civicrm-extension-status