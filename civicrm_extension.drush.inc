<?php

/**
 * Drush commands for performing extension-related tasks
 * andyw@circle, 27/09/2014
 */

function civicrm_extension_drush_command() {

    $items['civicrm-extension-disable'] = array(
        'callback'    => 'drush_civicrm_extension_disable',
        'description' => "Disable CiviCRM extension",
        'aliases'     => array('cvxdis', 'civicrm-xdis'),
    );

    $items['civicrm-extension-directory'] = array(
        'callback'    => 'drush_civicrm_extension_directory',
        'description' => "List extensions available from the extensions directory",
        'aliases'     => array('cvxdir', 'civicrm-xdir'),
    );

    $items['civicrm-extension-download'] = array(
        'callback'    => 'drush_civicrm_extension_download',
        'description' => "Disable CiviCRM extension",
        'aliases'     => array('cvxdl', 'civicrm-xdl'),
    );

    $items['civicrm-extension-enable'] = array(
        'callback'    => 'drush_civicrm_extension_enable',
        'description' => "Enable CiviCRM extension",
        'aliases'     => array('cvxen', 'civicrm-xen'),
    );

    $items['civicrm-extension-status'] = array(
        'callback'    => 'drush_civicrm_extension_status',
        'description' => "Show current status of site extensions",
        'aliases'     => array('cvxstat', 'civicrm-xstat'),
    );

    $items['civicrm-extension-uninstall'] = array(
        'callback'    => 'drush_civicrm_extension_uninstall',
        'description' => "Uninstall CiviCRM extension",
        'aliases'     => array('cvxun', 'civicrm-xun'),
    );

    return $items;

}

function civicrm_extension_drush_help($section) {
    
    switch ($section) {

        case 'drush:civicrm-extension-directory':
            return dt("List extensions available on the extensions directory");
    


    }

}

function drush_civicrm_extension_directory($search_query=null) {
    
    civicrm_initialize();
    drush_print("Querying extensions directory ...");

    if ($extensions = CRM_Extension_System::singleton()->getBrowser()->getExtensions()) {

        drush_print_table(
            array_map(function($extension) {
                return array(
                    $extension->key,
                    $extension->name,
                    $extension->type,
                    $extension->description
                );
            }, $extensions), true
        );

    } else drush_log(dt('Unable to get extensions'), 'error');

}

function drush_civicrm_extension_download($extension_key) {

    civicrm_initialize();
    
    $cache_dir  = CRM_Core_Config::singleton()->extensionsDir . 'cache';
    $cache_file = sprintf('%s/%s.xml', $cache_dir, $extension_key);

    # if no cache file exists, refresh cache
    if (!file_exists($cache_file)) {
        drush_print('No cache file found. Refreshing cache ...');
        CRM_Extension_System::singleton()->getBrowser()->getExtensions();
    }
    
    # if no cache file exists now, we have a problem
    if (!file_exists($cache_file)) {
        drush_log(dt("Unable to locate extension '!key' for this version of CiviCRM", array(
            '!key' => $extension_key
        )), 'error');
        return false;
    }

    # load info file, check has download url
    $info = CRM_Extension_Info::loadFromFile($cache_file);
    if (!isset($info->downloadUrl)) {
        drush_log(dt('Extension has no download url'), 'error');
        return false;
    }

    # download extension
    drush_print(dt('Downloading extension ...'));

    if (!CRM_Extension_System::singleton()->getDownloader()->download($extension_key, $info->downloadUrl)) {
        drush_log(dt('Unable to download extension'), 'error');
        return false;
    }

    drush_log(dt('Extension downloaded'), 'ok');
    return true;

}

function drush_civicrm_extension_disable($key) {

    civicrm_initialize();
    $manager = CRM_Extension_System::singleton()->getManager();

    switch ($status = $manager->getStatus($key)) {
        
        # if enabled, we can disable it
        case 'installed':

            drush_print(dt('Disabling extension...'));
            
            try {
                $manager->disable(array($key));
            } catch (CRM_Extension_Exception $e) {
                drush_log($e->getMessage(), 'error');
                return false;
            }

            drush_log(dt('Extension disabled'), 'ok');           
            return true;

        # it's already disabled
        case 'disabled':
            drush_log(dt('Extension is already disabled'), 'ok');
            return false;

        # we can't disable it
        default:
            drush_log(dt("Extension cannot be disabled. Current extension status is '!status'", array(
                '!status' => $status
            )), 'error');
            return false;
    
    }
    
}

function drush_civicrm_extension_enable($key) {

    civicrm_initialize();
    $manager = CRM_Extension_System::singleton()->getManager();

    # if extension not present, prompt to download it
    if ($manager->getStatus($key) == 'unknown') {

        if (!drush_confirm(dt('!key is not present. Attempt to locate and download it?', array('!key' => $key)))) 
            return drush_print(dt('Operation cancelled.'));
        
        if (!drush_civicrm_extension_download($key))
            return;
            
    }

    # check status again - in the event we just downloaded something, this
    # should now have changed
    switch ($status = $manager->getStatus($key)) {
        
        # can enable on either of the following statuses
        case 'uninstalled':
        case 'disabled':
            
            drush_print(dt('Enabling extension...'));
            
            try {
                $manager->enable(array($key));
            } catch (CRM_Extension_Exception $e) {
                drush_log($e->getMessage(), 'error');
            }

            drush_log(dt('Extension enabled'), 'ok');
            break;

        # it's already enabled
        case 'installed':
            return drush_log(dt('Extension is already installed and enabled'), 'ok');
        
        # arguable whether this will ever run (should exit function before getting to here in the
        # event of a problem), but just in case ..
        default:
            drush_log(dt('A problem occurred enabling the extension. Extension status is !status', array(
                '!status' => $status
            )));
            break;

    }

}

function drush_civicrm_extension_status() {

    civicrm_initialize();

    if ($extensions = CRM_Extension_System::singleton()->getManager()->getStatuses()) {
        
        $extension_list = array();

        $status_colors = array(
            'installed'         => 32,
            'uninstalled'       => 31,
            'disabled'          => 33,
            'unknown'           => 31,
            'installed-missing' => 31,
            'disabled-missing'  => 31
        );
        
        foreach ($extensions as $extension_key => $extension_status)
            $extension_list[] = array(
                CRM_Extension_System::singleton()
                    ->getMapper()
                    ->keyToInfo($extension_key)
                    ->name,
                $extension_key,
                sprintf("\033[%dm[%s]\033[0m", $status_colors[$extension_status], $extension_status)
            );

        drush_print();
        drush_print_table($extension_list);

    } else drush_log(dt('Unable to get extensions'), 'error');

}

function drush_civicrm_extension_uninstall($key) {

    civicrm_initialize();
    $manager = CRM_Extension_System::singleton()->getManager();

    if ($manager->getStatus($key) == 'installed') {

        if (!drush_confirm(dt('!key is installed and enabled. Disable and uninstall?', array('!key' => $key)))) 
            return drush_print(dt('Operation cancelled.'));
        
        if (!drush_civicrm_extension_disable($key))
            return;
    
    }

    # check status again - in the event we just disabled the extension, this
    # should now have changed to 'disabled'
    switch ($status = $manager->getStatus($key)) {
        
        # can uninstall
        case 'disabled':
            
            drush_print(dt('Uninstalling extension...'));
            
            try {
                $manager->uninstall(array($key));
            } catch (CRM_Extension_Exception $e) {
                drush_log($e->getMessage(), 'error');
            }

            drush_log(dt('Extension uninstalled'), 'ok');
            break;

        # it's already uninstalled
        case 'uninstalled':
        case 'unknown':
            return drush_log(dt('Extension is already uninstalled or not present'), 'ok');
        
        # arguable whether this will ever run (should exit function before getting to here in the
        # event of a problem), but just in case ..
        default:
            drush_log(dt('A problem occurred uninstalling the extension. Extension status is !status', array(
                '!status' => $status
            )));
            break;

    }

}