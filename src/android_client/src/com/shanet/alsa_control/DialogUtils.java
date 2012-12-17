// alsa-control-server
// shane tully (shane@shanetully.com)
// shanetully.com
// https://github.com/shanet/Alsa-Channel-Control

package com.shanet.alsa_control;

import java.io.File;
import java.io.IOException;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.text.util.Linkify;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public abstract class DialogUtils {

	public static void displayErrorDialog(Context context, int title, int message) {
		displayErrorDialog(context, context.getString(title), context.getString(message));
	}
	
	
	public static void displayErrorDialog(Context context, String title, String message) {
		// This function displays error messages with a given title and message.
		// It cuts down on duplicate code.
		new AlertDialog.Builder(context)
		.setTitle(title)
		.setMessage(message)
		.setIcon(R.drawable.error_icon)
		.setPositiveButton(R.string.okay, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int whichButton) {}
		})
		.show();
	}


	public static void showDeleteDialog(final Context context, final String content, final int position, final File contentIOFile, final int contentType) {
		// Show the dialog to confirm the deletion of content
		new AlertDialog.Builder(context)
		.setTitle((contentType == Constants.SERVERS) ? R.string.deleteServerTitle : R.string.deleteChannelTitle)
		.setMessage(String.format(context.getString(R.string.deleteDialog), content))
		.setIcon(R.drawable.error_icon)
		.setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int whichButton) {
				try {
					// Delete the content from the text file
					Utils.removeContent(context, contentIOFile, content);

					// Remove the content from the content list
					((SelectDialog)context).removeContent(content, position);
				} catch (IOException e) {
					DialogUtils.displayErrorDialog(context, R.string.writeIOErrorTitle, R.string.writeIOError);
				}
			}
		})
		.setNegativeButton(R.string.nope, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int whichButton) {}
		})
		.show();
	}
	
	
	public static void showAddContentDialog(final Context context, final File contentIOFile, final int contentType) {
		LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View layout = inflater.inflate(R.layout.add_content_dialog, null);
		
		// Set the appropriate text on the label
		((TextView) layout.findViewById(R.id.addContentLabel)).setText((contentType == Constants.SERVERS) ? R.string.serverLabel : R.string.channelLabel);
		
		final EditText contentInput = (EditText) layout.findViewById(R.id.addContentEdit);

		new AlertDialog.Builder(context)
			.setTitle((contentType == Constants.SERVERS) ? R.string.addServer : R.string.addChannel)
			.setView(layout)
			.setIcon(R.drawable.edit_icon)
			.setPositiveButton(R.string.okay, new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
					if(!contentInput.getText().toString().equals("")) {
						try {
							// Add the new content to the text file
							Utils.addContent(contentIOFile, contentInput.getText().toString());
							
							// Add the new content to the content list
							((SelectDialog)context).addContent(contentInput.getText().toString());
						} catch (IOException e) {
							DialogUtils.displayErrorDialog(context, R.string.writeIOErrorTitle, R.string.writeIOError);
						}
					} else {
						Toast.makeText(context, ((contentType == Constants.SERVERS) ? R.string.emptyAddServer : R.string.emptyAddChannel), Toast.LENGTH_LONG).show();
					}
				}
			})
			.setNegativeButton(R.string.nevermind, new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {}
			})
			.show();
	}
	
	
	public static void showEditPortDialog(final Context context) {
		LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View layout = inflater.inflate(R.layout.add_content_dialog, null);
		
		// Set the appropriate text on the label
		((TextView) layout.findViewById(R.id.addContentLabel)).setText(context.getString(R.string.portLabel));
		
		// Set the edit text to the current port
		final EditText portInput = (EditText) layout.findViewById(R.id.addContentEdit);
		portInput.setText(Integer.valueOf(Utils.getIntPref(context, "port")).toString());

		new AlertDialog.Builder(context)
			.setTitle(context.getString(R.string.editPort))
			.setView(layout)
			.setIcon(R.drawable.edit_icon)
			.setPositiveButton(R.string.okay, new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {
					if(!portInput.getText().toString().equals("")) {
						Utils.writeIntPref(context, "port", Integer.parseInt(portInput.getText().toString()));
					} else {
						Toast.makeText(context, R.string.emptyPort, Toast.LENGTH_LONG).show();
					}
				}
			})
			.setNegativeButton(R.string.nevermind, new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int whichButton) {}
			})
			.show();
	}

	
	public static void showAboutDialog(Context context, int dialogType) {
		createAboutDialog(context, dialogType).show();
	}

	
	public static AlertDialog createAboutDialog(Context context, int dialogType) {
		// Create the given about dialog type
		AlertDialog.Builder dialog = new AlertDialog.Builder(context);
		View aboutLayout = LayoutInflater.from(context).inflate(R.layout.about_dialog, null);
		TextView aboutText = (TextView) aboutLayout.findViewById(R.id.aboutText);

		dialog.setView(aboutLayout);
		dialog.setIcon(R.drawable.info_icon);
		//((ImageView)aboutLayout.findViewById(R.id.aboutLogo)).setImageDrawable(context.getResources().getDrawable(R.drawable.full_logo));

		dialog.setPositiveButton(R.string.close, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int whichButton) {}
		});

		// Determine what dialog type is to be shown
		switch(dialogType) {
		case Constants.ABOUT_THIS_APP:
			dialog.setTitle(R.string.aboutThisAppTitle);
			aboutText.setText(R.string.aboutThisAppText);
			break;
		case Constants.CHANGELOG:
			dialog.setTitle(R.string.changelogTitle);
			aboutText.setText(R.string.changelogText);
			break;
		default:
			return null;
		}

		// Make links out all URL's and email's in the dialog
		// Except the release notes. The linker recognizes
		// 4 digit version numbers as IP addresses
		if(dialogType != Constants.CHANGELOG)
			Linkify.addLinks(aboutText, Linkify.ALL);

		return dialog.create();
	}
}