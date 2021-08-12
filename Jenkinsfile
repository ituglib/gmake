/*
 * Pipeline to build NSGit's GMAKEN on a Windows node.
 * This pipeline should work on any J06.22 or L19.02 system or above, providing
 * the GMAKE-BUILD-OPTIONS.name exists to deal with hard-coding requirements
 * and has no hard-coding requirements for a particular agent. The job requires
 * access to the 'apache' class node for doxygen updates, 'glass' node for the
 * official build and 'nonstop' for the test node (when that is turned on).
 *
 * Extensions, at some point, we may get both J and L, which would require a
 * duplicate build/test step on the two nodes and differentiate 'nonstop' into
 * 'nonstop-J' and 'nonstop-L'.
 */
 pipeline {
	agent none
	options {
		buildDiscarder(logRotator(numToKeepStr: '2'))
	}
	stages {
		stage('documentation') {
			when {
				branch 'development'
			}
			steps {
				lock(resource: 'gmakedoxygen') {
				    node('apache') {
				        checkout([$class: 'GitSCM',
				            branches: [[name: '*/development']],
				            extensions: [
				            	[$class: 'CleanBeforeCheckout'],
				            	[$class: 'SubmoduleOption', disableSubmodules: false, parentCredentials: true,
				            		recursiveSubmodules: true, reference: '', trackingSubmodules: false]],
				            	doGenerateSubmoduleConfigurations: false, submoduleCfg: [],
				            userRemoteConfigs: [[credentialsId: 'Randall', url: 'git@bitbucket.org:nonstop-git/make.git']]])
				        sh 'sed -i -r -e "1,\\$s/^(PROJECT_NUMBER +)= .*\\$/\\1= `git describe`/" doxyfile'
				        sh 'git update-index --assume-unchanged doxyfile'
				        sh 'doxygen doxyfile || echo "Ignoring errors"'
				        sh 'rm -rf /var/www/http/Products/GMake/*'
				        sh 'cp -rp docs/html /var/www/html/Products/GMake/'
			        }
		        }
		    }
		}
		stage('development') {
			when {
				anyOf { branch 'development' }
			}
			steps {
				lock(resource: 'make-test-environment') {
				    node('nonstop') {
				        checkout([$class: 'GitSCM',
				            branches: [[name: '*/development']],
				            extensions: [
				            	[$class: 'CleanBeforeCheckout'],
				            	[$class: 'SubmoduleOption', disableSubmodules: false, parentCredentials: true,
				            		recursiveSubmodules: true, reference: '', trackingSubmodules: false]],
				            	doGenerateSubmoduleConfigurations: false, submoduleCfg: [],
				            userRemoteConfigs: [[credentialsId: 'Randall', url: 'git@bitbucket.org:nonstop-git/make.git']]])
				        sh 'make -f Makefile.NSE.OSS'
			        }
		        }
		    }
		}
		stage('test') {
			when {
				anyOf { branch 'development' }
			}
			steps {
				lock(resource: 'make-test-environment') {
				    node('nonstop') {
				        checkout([$class: 'GitSCM',
				            branches: [[name: '*/development']],
				            extensions: [
				            	[$class: 'SubmoduleOption', disableSubmodules: false, parentCredentials: true,
				            		recursiveSubmodules: true, reference: '', trackingSubmodules: false]],
				            	doGenerateSubmoduleConfigurations: false, submoduleCfg: [],
				            userRemoteConfigs: [[credentialsId: 'Randall', url: 'git@bitbucket.org:nonstop-git/make.git']]])
				       	dir('test') {
							sh 'make' 
					    }
				    }
			    }
		    }
		}
		stage('release') {
			when {
				branch 'production'
			}
			steps {
				lock(resource: 'gmakeprod-build') {
					node('glass') {
				        checkout([$class: 'GitSCM',
				            branches: [[name: '*/production']],
				            extensions: [
				            	[$class: 'CleanBeforeCheckout'],
				            	[$class: 'SubmoduleOption', disableSubmodules: false, parentCredentials: true,
				            		recursiveSubmodules: true, reference: '', trackingSubmodules: false]],
				            	doGenerateSubmoduleConfigurations: false, submoduleCfg: [],
				            userRemoteConfigs: [[credentialsId: 'Randall', url: 'git@bitbucket.org:nonstop-git/make.git']]])
			        	withEnv(['COMP_ROOT=C:\\Program Files (x86)\\HPE NonStop\\J06.20',
			        		'NONSTOPOSVERSION=J06.20',
			        		'NSDEE_SYS_INCLUDE_PATH=C:\\Program Files (x86)\\HPE NonStop\\J06.20\\usr\\include',
			        		'NSDEE_SYS_INCLUDE_PATH_ESC=C:\\\\Program Files (x86)\\\\HPE NonStop\\\\J06\\.20\\\\usr\\\\include',
			        		'PATH+WHATEVER=C:\\Program Files (x86)\\HPE NonStop\\J06.20\\usr\\bin;C:\\cygwin\\bin;']) {
			        		
			        		echo 'About to start the J-series production build'
				            sh 'make -f Makefile.NSE.Win clean'
				            sh 'make -f Makefile.NSE.Win GMAKE_EXE=../gmaken.J.txe POBJ_DLL=../gmakeext.J.dll COPYLIB_DLL=../gmakecpy.J.dll DDLDICT_DLL=../gmakeddl.J.dll'
					    }
			        	withEnv(['COMP_ROOT=C:\\Program Files (x86)\\HPE NonStop\\L16.05',
			        		'NONSTOPOSVERSION=L16.05',
			        		'NSDEE_SYS_INCLUDE_PATH=C:\\Program Files (x86)\\HPE NonStop\\L16.05\\usr\\include',
			        		'NSDEE_SYS_INCLUDE_PATH_ESC=C:\\\\Program Files (x86)\\\\HPE NonStop\\\\L16\\.05\\\\usr\\\\include',
			        		'PATH+WHATEVER=C:\\Program Files (x86)\\HPE NonStop\\L16.05\\usr\\bin;C:\\cygwin\\bin;']) {
			        		
				            sh 'make -f Makefile.NSX.Win clean'
				            sh 'make -f Makefile.NSX.Win GMAKE_EXE=../gmaken.L.txe POBJ_DLL=../gmakeext.L.dll COPYLIB_DLL=../gmakecpy.L.dll DDLDICT_DLL=../gmakeddl.L.dll'
					    }
					}
				}
			}
		}
	}
	post {
        always {
            mail bcc: '', body: "${currentBuild.currentResult}: Job ${env.JOB_NAME} build ${env.BUILD_NUMBER}\nDuration: ${currentBuild.durationString}\nMore info at: ${env.BUILD_URL}", cc: '', from: 'rsbecker@nexbridge.com', replyTo: '', subject: "[Jenkins nonstop] ${currentBuild.currentResult}: job ${env.JOB_NAME}", to: 'rsbecker@nexbridge.com'
        }
	}
}