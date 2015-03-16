#include "AutosaveJob.h"
#include "../SaveHandler.h"

#include "../Control.h"

AutosaveJob::AutosaveJob(Control* control)
{
	XOJ_INIT_TYPE(AutosaveJob);

	this->control = control;
}

AutosaveJob::~AutosaveJob()
{
	XOJ_RELEASE_TYPE(AutosaveJob);
}

void AutosaveJob::afterRun()
{
	XOJ_CHECK_TYPE(AutosaveJob);

	GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) control->getWindow(),
	                                           GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Autosave: %s"),
	                                           StringUtils::c_str(this->error));
	gtk_window_set_transient_for(GTK_WINDOW(dialog),
	                             GTK_WINDOW(this->control->getWindow()->getWindow()));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void AutosaveJob::run()
{
	XOJ_CHECK_TYPE(AutosaveJob);

	SaveHandler handler;

	control->getUndoRedoHandler()->documentAutosaved();

	Document* doc = control->getDocument();

	doc->lock();
	handler.prepareSave(doc);
	String filename = doc->getFilename();
	doc->unlock();

	// TODO: incrementel autosave
	if (filename.isEmpty())
	{
		filename = Util::getAutosaveFilename();
	}
	else
	{
		int pos = filename.lastIndexOf("/") + 1;
		String folder = String(filename).retainBetween(0, pos);
		String file = String(filename).retainBetween(pos);
		filename = folder + ".";
		if (file.length() > 5 && String(file).retainBetween(file.length()-4) == ".xoj")
		{
			filename += String(file).retainBetween(0,file.length()-4);
		}
		filename += ".autosave.xoj";
	}

	control->renameLastAutosaveFile();

	GzOutputStream* out = new GzOutputStream(filename);
	handler.saveTo(out, filename);
	delete out;

	this->error = handler.getErrorMessage();
	if (!this->error.isEmpty())
	{
		callAfterRun();
	}
	else
	{
		//control->deleteLastAutosaveFile(filename);
		control->setLastAutosaveFile(filename);
	}
}

JobType AutosaveJob::getType()
{
	XOJ_CHECK_TYPE(AutosaveJob);
	return JOB_TYPE_AUTOSAVE;
}

