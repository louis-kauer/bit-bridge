#ifndef BIT_BRIDGE_MAIN_FRAME_HPP
#define BIT_BRIDGE_MAIN_FRAME_HPP

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <memory>
#include "LoadBalancerConfig.hpp"
#include "IConfigSerializer.hpp"
#include "AppSettings.hpp"

enum class BitBridgeIds : int {
    ID_LOWEST = 0,
    ID_ADD_SERVICE,
    ID_REMOVE_SERVICE,
    ID_SAVE_CONFIG,
    ID_LOAD_CONFIG
};

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size,
              std::unique_ptr<AppSettings> settings);

private:
    void OnAddService(wxCommandEvent &event);

    void OnRemoveService(wxCommandEvent &event);

    void OnSaveConfig(wxCommandEvent &event);

    void OnLoadConfig(wxCommandEvent &event);

    void HandleTabNavigation(wxKeyEvent &event);

    void RepositionEmptyLabel() const;

    void RefreshServiceList() const;

    void LoadExistingConfig();

    void ApplyDefaultConfig() const;

    void MarkUnsaved();

    void MarkSaved();

    static bool ValidateInput(const wxString &name, const wxString &ip, const wxString &port, const wxString &weight);

    std::unique_ptr<AppSettings> m_settings;
    std::unique_ptr<LoadBalancerConfig> m_config;
    std::unique_ptr<IConfigSerializer> m_serializer;

    wxListCtrl *m_serviceList;
    wxTextCtrl *m_nameInput;
    wxTextCtrl *m_ipInput;
    wxTextCtrl *m_portInput;
    wxTextCtrl *m_weightInput;
    wxButton *m_addButton;
    wxButton *m_removeButton;
    wxButton *m_saveButton;
    wxButton *m_loadButton;
    wxStaticText *m_emptyLabel;
    wxStaticText *m_unsavedLabel;
    bool m_hasUnsavedChanges;
};

#endif //BIT_BRIDGE_MAIN_FRAME_HPP